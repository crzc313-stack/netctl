#include "netctl.h"

#include <stdio.h>
#include <string.h>

static void usage(const char *program)
{
    fprintf(stderr, "Usage: %s dns <domain> [server1 server2 ...]\n", program);
}

int main(int argc, char **argv)
{
    static const char *default_servers[] = {
        "1.1.1.1", "8.8.8.8", "9.9.9.9", "208.67.222.222",
        "94.140.14.14", "185.222.222.222", "64.6.64.6", "77.88.8.8"
    };
    const char **servers;
    int count;

    if (argc < 3 || strcmp(argv[1], "dns") != 0) {
        usage(argv[0]);
        return 1;
    }
    if (argc == 3) {
        servers = default_servers;
        count = (int)(sizeof(default_servers) / sizeof(default_servers[0]));
    } else {
        servers = (const char **)&argv[3];
        count = argc - 3;
    }
    dns_bench(servers, count, argv[2], 1);
    return 0;
}