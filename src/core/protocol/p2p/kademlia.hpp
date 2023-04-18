#pragma once

/*#ifdef _WIN32
#include <Winsock2.h>
#else
#include <sys/socket.h> // sockaddr_storage
#endif

#include <cstddef> // size_t
#include <cstdio> // FILE*
#include <ctime> // time_t*/

// Copied from dht-example.c
/* For crypt */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <sys/signal.h>

/* This must be provided by the user. */
int dht_sendto(int sockfd, const void *buf, int len, int flags,
               const struct sockaddr *to, int tolen);
int dht_blacklisted(const struct sockaddr *sa, int salen);
void dht_hash(void *hash_return, int hash_size,
              const void *v1, int len1,
              const void *v2, int len2,
              const void *v3, int len3);
int dht_random_bytes(void *buf, size_t size);


namespace neroshop {

// Copied from dht-example.c

static void
sigdump(int signo);

static void
sigtest(int signo);

static void
sigexit(int signo);

static void
init_signals(void);

const unsigned char hash[20] = {
    0x54, 0x57, 0x87, 0x89, 0xdf, 0xc4, 0x23, 0xee, 0xf6, 0x03,
    0x1f, 0x81, 0x94, 0xa9, 0x3a, 0x16, 0x98, 0x8b, 0x72, 0x7b
};

/* The call-back function is called by the DHT whenever something
   interesting happens.  Right now, it only happens when we get a new value or
   when a search completes, but this may be extended in future versions. */
void dht_callback(void *closure,
         int event,
         const unsigned char *info_hash,
         const void *data, size_t data_len);

static unsigned char buf[4096];

static int
set_nonblocking(int fd, int nonblocking);

}
