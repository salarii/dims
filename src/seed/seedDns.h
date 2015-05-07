// @2011 - @2014 sipa

#ifndef SEED_DNS_H_
#define SEED_DNS_H_ 1

#include <stdint.h>

typedef struct {
    int v;
    union {
       unsigned char v4[4];
       unsigned char v6[16];
    } data;
} addr_t;

typedef struct {
  int port;
  int datattl;
  int nsttl;
  const char *host;
  const char *ns;
  const char *mbox;
  int (*cb)(void *opt, addr_t *addr, unsigned int max, int ipv4, int ipv6);
  // stats
  uint64_t nRequests;
} dns_opt_t;

extern int dnsserver(dns_opt_t *opt);

#endif
