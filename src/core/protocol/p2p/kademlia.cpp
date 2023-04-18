#include "kademlia.hpp"

#include <netinet/in.h>
#include <arpa/inet.h>

#include <fstream>
#include <random>
#include <stdexcept>
#include <cstring> // memset

#include <openssl/evp.h>

/* Functions called by the DHT. */

int dht_sendto(int sockfd, const void *buf, int len, int flags,
               const struct sockaddr *to, int tolen) {
    return sendto(sockfd, buf, len, flags, to, tolen);               
}

int dht_blacklisted(const struct sockaddr *sa, int salen) {
    return 0; // Don't blacklist any nodes for now
}

void dht_hash(void *hash_return, int hash_size,
              const void *v1, int len1,
              const void *v2, int len2,
              const void *v3, int len3) {
    EVP_MD_CTX* ctx;
    const EVP_MD* md;
    unsigned int md_len;

    ctx = EVP_MD_CTX_new();
    md = EVP_md5();
    EVP_DigestInit_ex(ctx, md, NULL);
    EVP_DigestUpdate(ctx, v1, len1);
    EVP_DigestUpdate(ctx, v2, len2);
    EVP_DigestUpdate(ctx, v3, len3);
    EVP_DigestFinal_ex(ctx, (unsigned char*)hash_return, &md_len);
    EVP_MD_CTX_free(ctx);

    if (hash_size > 16) {
        memset((char*)hash_return + 16, 0, hash_size - 16);
    }
}
              
#if !defined(__gnu_linux__)
int dht_random_bytes(void* buf, size_t size) {
    static std::random_device rdev;
    static std::mt19937_64 engine(rdev());
    std::uniform_int_distribution<uint8_t> dist;
    uint8_t* bytes = reinterpret_cast<uint8_t*>(buf);
    for (size_t i = 0; i < size; ++i) {
        bytes[i] = dist(engine);
    }
    return size;
}
#else
int dht_random_bytes(void* buf, size_t size) {
    std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
    if (!urandom)
        throw std::runtime_error("Failed to open /dev/urandom");

    urandom.read(static_cast<char*>(buf), size);
    if (!urandom)
        throw std::runtime_error("Failed to read from /dev/urandom");

    return static_cast<int>(size);
}
#endif
// See dht-example.c for different implementation examples
// These functions may be defined in dht.c but have NEVER been declared in dht.h
/*int
neroshop::dht_send(const void *buf, size_t len, int flags,
         const struct sockaddr *sa, int salen) {*/
/*    int s;

    if(salen == 0)
        abort();

    if(node_blacklisted(sa, salen)) {
        debugf("Attempting to send to blacklisted node.\n");
        errno = EPERM;
        return -1;
    }

    if(sa->sa_family == AF_INET)
        s = dht_socket;
    else if(sa->sa_family == AF_INET6)
        s = dht_socket6;
    else
        s = -1;

    if(s < 0) {
        errno = EAFNOSUPPORT;
        return -1;
    }

    return dht_sendto(s, buf, len, flags, sa, salen);*/
//}

/*int
neroshop::node_blacklisted(const struct sockaddr *sa, int salen) {*/
/*    int i;

    if((unsigned)salen > sizeof(struct sockaddr_storage))
        abort();

    if(dht_blacklisted(sa, salen))
        return 1;

    for(i = 0; i < DHT_MAX_BLACKLISTED; i++) {
        if(memcmp(&blacklist[i], sa, salen) == 0)
            return 1;
    }

    return 0;*/
//}




// Copied from dht-example.c

#define MAX_BOOTSTRAP_NODES 20
static struct sockaddr_storage bootstrap_nodes[MAX_BOOTSTRAP_NODES];
static int num_bootstrap_nodes = 0;

static volatile sig_atomic_t dumping = 0, searching = 0, exiting = 0;

void
neroshop::sigdump(int signo)
{
    dumping = 1;
}

void
neroshop::sigtest(int signo)
{
    searching = 1;
}

void
neroshop::sigexit(int signo)
{
    exiting = 1;
}

void
neroshop::init_signals(void)
{
    struct sigaction sa;
    sigset_t ss;

    sigemptyset(&ss);
    sa.sa_handler = sigdump;
    sa.sa_mask = ss;
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    sigemptyset(&ss);
    sa.sa_handler = sigtest;
    sa.sa_mask = ss;
    sa.sa_flags = 0;
    sigaction(SIGUSR2, &sa, NULL);

    sigemptyset(&ss);
    sa.sa_handler = sigexit;
    sa.sa_mask = ss;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

void
neroshop::dht_callback(void *closure,
         int event,
         const unsigned char *info_hash,
         const void *data, size_t data_len) {
/*    if(event == DHT_EVENT_SEARCH_DONE)
        printf("Search done.\n");
    else if(event == DHT_EVENT_SEARCH_DONE6)
        printf("IPv6 search done.\n");
    else if(event == DHT_EVENT_VALUES)
        printf("Received %d values.\n", (int)(data_len / 6));
    else if(event == DHT_EVENT_VALUES6)
        printf("Received %d IPv6 values.\n", (int)(data_len / 18));
    else
        printf("Unknown DHT event %d.\n", event);*/
}

int
neroshop::set_nonblocking(int fd, int nonblocking)
{
    int rc;
    rc = fcntl(fd, F_GETFL, 0);
    if(rc < 0)
        return -1;

    rc = fcntl(fd, F_SETFL,
               nonblocking ? (rc | O_NONBLOCK) : (rc & ~O_NONBLOCK));
    if(rc < 0)
        return -1;

    return 0;
}         















