#ifndef NETCTL_H
#define NETCTL_H

#define MAX_DNS_SERVERS 16
#define DEFAULT_TIMEOUT_MS 2000

struct dns_result {
    char server[64];
    double latency_ms;
    int success;
    char response[256];
};

int dns_query(const char *server, const char *domain, struct dns_result *out);
void dns_bench(const char **servers, int count, const char *domain, int parallel);

#endif