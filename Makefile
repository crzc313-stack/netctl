CC = gcc

CFLAGS = -O2 -Wall -Wextra -Iinclude -lpthread

TARGET = netctl

SRC = src/main.c src/dns/resolver.c src/dns/bench.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) test_dns

test:
	$(CC) $(CFLAGS) -o test_dns tests/test_dns.c src/dns/resolver.c && ./test_dns

.PHONY: clean test