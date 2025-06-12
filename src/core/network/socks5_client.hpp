#pragma once

#ifndef SOCKS5_CLIENT_HPP_NEROSHOP
#define SOCKS5_CLIENT_HPP_NEROSHOP

#include <cstddef>
#include <cstdint> // uint16_t
#if defined(__gnu_linux__)
#include <sys/types.h> // ssize_t
#endif
#include <memory> // std::unique_ptr
#include <string>

inline constexpr const char* TOR_SOCKS5_HOST = "127.0.0.1";
inline constexpr uint16_t TOR_SOCKS5_PORT    = 9050;

namespace neroshop {

class OnionAddressGenerator;

class Socks5Client {
public:
    Socks5Client(const char* socks_host = "127.0.0.1", uint16_t socks_port = 9050, bool tor_hidden_service = false);
    ~Socks5Client();
    
    void connect(const char* dest_host, uint16_t dest_port);
    ssize_t send(const void* buf, size_t len, int flags = 0);
    ssize_t recv(void* buf, size_t len, int flags = 0);
    void adopt_socket(int fd);
    
    int get_socket() const;
    std::string get_onion_address() const;
    uint16_t get_port() const;
    
private:
    bool socks5_handshake(const char* dest_host, uint16_t dest_port);
    bool socks5_handshake_auth(const char* dest_host, uint16_t dest_port);
    const char* socks_host_;
    uint16_t socks_port_;
    int sockfd_;
    std::string onion_address_;
    std::unique_ptr<OnionAddressGenerator> onion_gen_;
    uint16_t port_; // client port
};

}

#endif
