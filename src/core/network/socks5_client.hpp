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
#include <atomic>

inline constexpr const char* TOR_SOCKS5_HOST = "127.0.0.1";
inline constexpr uint16_t TOR_SOCKS5_PORT    = 9050;

namespace neroshop {

class TorManager;

class Socks5Client {
public:
    explicit Socks5Client(const char* host = "127.0.0.1", uint16_t port = 9050,
                 std::shared_ptr<neroshop::TorManager> tor_manager = nullptr);
    ~Socks5Client();
    
    void connect(const char* dest_host, uint16_t dest_port);
    ssize_t send(const void* buf, size_t len, int flags = 0);
    ssize_t recv(void* buf, size_t len, int flags = 0);
    void adopt_socket(int fd); // ???
    void close();
    
    void set_tor_manager(std::shared_ptr<neroshop::TorManager> tor_manager);
    
    int get_socket() const noexcept;
    uint16_t get_socks_port() const noexcept;
    int get_tor_progress() const noexcept;
    std::string get_tor_address() const noexcept;
    std::shared_ptr<TorManager> get_tor_manager() const noexcept;
    uint16_t get_port() const noexcept;
    
    bool is_connected() const;
    
private:
    bool socks5_handshake(const char* dest_host, uint16_t dest_port);
    bool socks5_handshake_auth(const char* dest_host, uint16_t dest_port);
    uint16_t reserve_available_port(uint16_t preferred_port);
    std::string socks_host_;
    uint16_t socks_port_;
    int sockfd_ = -1;
    std::shared_ptr<neroshop::TorManager> tor_manager;
    uint16_t port_ = 0; // Client P2P port / HiddenServicePort (not SocksPort)
    std::atomic<bool> connected_{false};
    
    static constexpr int CONNECT_TIMEOUT_MS = 10000;
    static constexpr int IO_TIMEOUT_MS = 30000;
    static constexpr size_t MAX_DOMAIN_LEN = 255;

    // Production-grade I/O helpers
    ssize_t send_all(const void* buf, size_t len, int flags, int timeout_ms);
    ssize_t recv_all(void* buf, size_t len, int flags, int timeout_ms);
    bool set_socket_timeout(int timeout_ms);
    std::string socks5_error(uint8_t rep_code);
};

}

#endif
