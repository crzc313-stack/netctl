#include "netctl.h"

#include <stdio.h>

int main(void)
{
    struct dns_result result;

    if (dns_query("1.1.1.1", "example.com", &result) == 0 && result.success == 1) {
        printf("PASS: resolved example.com via %s in %.2f ms\n", result.server, result.latency_ms);
        return 0;
    }
    printf("FAIL: DNS query via %s (%s)\n", result.server, result.response);
    return 1;
}