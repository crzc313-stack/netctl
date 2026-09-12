#include "netctl.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static double elapsed_ms(const struct timespec *start, const struct timespec *end)
{
    return (double)(end->tv_sec - start->tv_sec) * 1000.0 +
           (double)(end->tv_nsec - start->tv_nsec) / 1000000.0;
}

static int encode_name(const char *domain, unsigned char *packet, size_t capacity, size_t *offset)
{
    const char *label = domain;

    while (*label != '\0') {
        const char *dot = strchr(label, '.');
        size_t length = dot == NULL ? strlen(label) : (size_t)(dot - label);
        if (length == 0 || length > 63 || *offset + length + 1 >= capacity) {
            return -1;
        }
        packet[(*offset)++] = (unsigned char)length;
        memcpy(packet + *offset, label, length);
        *offset += length;
        if (dot == NULL) {
            break;
        }
        label = dot + 1;
    }
    packet[(*offset)++] = 0;
    return 0;
}

static int skip_name(const unsigned char *packet, size_t length, size_t *offset)
{
    size_t jumps = 0;

    while (*offset < length && jumps++ < length) {
        unsigned char size = packet[(*offset)++];
        if (size == 0) {
            return 0;
        }
        if ((size & 0xc0) == 0xc0) {
            if (*offset >= length) {
                return -1;
            }
            (*offset)++;
            return 0;
        }
        if ((size & 0xc0) != 0 || *offset + size > length) {
            return -1;
        }
        *offset += size;
    }
    return -1;
}

int dns_query(const char *server, const char *domain, struct dns_result *out)
{
    unsigned char packet[512] = {0};
    unsigned char answer[2048] = {0};
    struct addrinfo hints = {0};
    struct addrinfo *addresses = NULL;
    struct timespec start, end;
    struct timeval timeout = { .tv_sec = DEFAULT_TIMEOUT_MS / 1000,
                               .tv_usec = (DEFAULT_TIMEOUT_MS % 1000) * 1000 };
    size_t offset = 12;
    size_t packet_length;
    size_t answer_size = 0;
    int socket_fd = -1;
    int error_code;
    unsigned short query_id = (unsigned short)(getpid() ^ (unsigned short)time(NULL));

    if (server == NULL || domain == NULL || out == NULL) {
        return -1;
    }
    memset(out, 0, sizeof(*out));
    snprintf(out->server, sizeof(out->server), "%s", server);

    packet[0] = (unsigned char)(query_id >> 8);
    packet[1] = (unsigned char)query_id;
    packet[2] = 0x01;
    packet[5] = 0x01;
    if (encode_name(domain, packet, sizeof(packet), &offset) != 0 || offset + 4 > sizeof(packet)) {
        snprintf(out->response, sizeof(out->response), "invalid domain");
        return -1;
    }
    packet[offset++] = 0;
    packet[offset++] = 1;
    packet[offset++] = 0;
    packet[offset++] = 1;
    packet_length = offset;

    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;
    error_code = getaddrinfo(server, "53", &hints, &addresses);
    if (error_code != 0) {
        snprintf(out->response, sizeof(out->response), "server: %s", gai_strerror(error_code));
        return -1;
    }

    for (struct addrinfo *address = addresses; address != NULL; address = address->ai_next) {
        ssize_t received;
        socket_fd = socket(address->ai_family, SOCK_DGRAM, 0);
        if (socket_fd < 0) {
            continue;
        }
        setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        clock_gettime(CLOCK_MONOTONIC, &start);
        if (sendto(socket_fd, packet, packet_length, 0, address->ai_addr, address->ai_addrlen) < 0) {
            clock_gettime(CLOCK_MONOTONIC, &end);
            snprintf(out->response, sizeof(out->response), "%s", strerror(errno));
            out->latency_ms = elapsed_ms(&start, &end);
            close(socket_fd);
            socket_fd = -1;
            continue;
        }
        received = recvfrom(socket_fd, answer, sizeof(answer), 0, NULL, NULL);
        clock_gettime(CLOCK_MONOTONIC, &end);
        out->latency_ms = elapsed_ms(&start, &end);
        if (received < 0) {
            snprintf(out->response, sizeof(out->response), "%s", errno == EAGAIN || errno == EWOULDBLOCK ? "timeout" : strerror(errno));
            close(socket_fd);
            socket_fd = -1;
            continue;
        }
        answer_size = (size_t)received;
        close(socket_fd);
        socket_fd = -1;
        break;
    }
    freeaddrinfo(addresses);
    if (answer_size < 12) {
        if (out->response[0] == '\0') {
            snprintf(out->response, sizeof(out->response), "unreachable");
        }
        return -1;
    }

    {
        unsigned short flags = (unsigned short)((answer[2] << 8) | answer[3]);
        unsigned short answers = (unsigned short)((answer[6] << 8) | answer[7]);
        size_t answer_offset = 12;
        if ((flags & 0x000f) != 0) {
            snprintf(out->response, sizeof(out->response), "DNS error (rcode %u)", flags & 0x000f);
            return -1;
        }
        if (skip_name(answer, answer_size, &answer_offset) != 0 || answer_offset + 4 > answer_size) {
            snprintf(out->response, sizeof(out->response), "malformed response");
            return -1;
        }
        answer_offset += 4;
        for (unsigned short index = 0; index < answers && answer_offset < answer_size; index++) {
            unsigned short type;
            unsigned short class_code;
            unsigned short data_length;
            if (skip_name(answer, answer_size, &answer_offset) != 0 || answer_offset + 10 > answer_size) {
                break;
            }
            type = (unsigned short)((answer[answer_offset] << 8) | answer[answer_offset + 1]);
            class_code = (unsigned short)((answer[answer_offset + 2] << 8) | answer[answer_offset + 3]);
            data_length = (unsigned short)((answer[answer_offset + 8] << 8) | answer[answer_offset + 9]);
            answer_offset += 10;
            if (answer_offset + data_length > answer_size) {
                break;
            }
            if (type == 1 && class_code == 1 && data_length == 4 &&
                inet_ntop(AF_INET, answer + answer_offset, out->response, sizeof(out->response)) != NULL) {
                out->success = 1;
                return 0;
            }
            answer_offset += data_length;
        }
    }
    snprintf(out->response, sizeof(out->response), "no A record");
    return -1;
}