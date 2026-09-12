#include "netctl.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct query_job {
    const char *server;
    const char *domain;
    struct dns_result result;
};

static void *run_query(void *argument)
{
    struct query_job *job = argument;
    dns_query(job->server, job->domain, &job->result);
    return NULL;
}

static int compare_results(const void *left, const void *right)
{
    const struct query_job *a = left;
    const struct query_job *b = right;
    if (a->result.success != b->result.success) {
        return b->result.success - a->result.success;
    }
    if (a->result.success && a->result.latency_ms != b->result.latency_ms) {
        return a->result.latency_ms < b->result.latency_ms ? -1 : 1;
    }
    return strcmp(a->server, b->server);
}

void dns_bench(const char **servers, int count, const char *domain, int parallel)
{
    struct query_job jobs[MAX_DNS_SERVERS];
    pthread_t threads[MAX_DNS_SERVERS];
    int thread_count = 0;
    int fastest = -1;

    if (servers == NULL || domain == NULL || count <= 0) {
        return;
    }
    if (count > MAX_DNS_SERVERS) {
        count = MAX_DNS_SERVERS;
    }
    for (int index = 0; index < count; index++) {
        jobs[index].server = servers[index];
        jobs[index].domain = domain;
        if (parallel && pthread_create(&threads[thread_count], NULL, run_query, &jobs[index]) == 0) {
            thread_count++;
        } else {
            run_query(&jobs[index]);
        }
    }
    for (int index = 0; index < thread_count; index++) {
        pthread_join(threads[index], NULL);
    }
    qsort(jobs, (size_t)count, sizeof(jobs[0]), compare_results);
    for (int index = 0; index < count; index++) {
        if (jobs[index].result.success) {
            fastest = index;
            break;
        }
    }

    printf("\n DNS benchmark for %s\n\n", domain);
    printf(" #  %-19s %10s   %s\n", "SERVER", "LATENCY", "STATUS");
    for (int index = 0; index < count; index++) {
        const char *color = jobs[index].result.success ? "\033[32m" : "\033[31m";
        const char *bold = index == fastest ? "\033[1m" : "";
        const char *reset = "\033[0m";
        if (jobs[index].result.success) {
             printf("%s%s%2d  %-19s %8.2f ms   %sOK%s\n", bold, color, index + 1,
                 jobs[index].result.server, jobs[index].result.latency_ms, color, reset);
        } else {
            printf("%s%s%2d  %-19s %10s   FAIL (%s)%s\n", bold, color, index + 1,
                   jobs[index].result.server, "--", jobs[index].result.response, reset);
        }
    }
}