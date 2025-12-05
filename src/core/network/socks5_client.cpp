#include "socks5_client.hpp"

#include "tor_manager.hpp"
#include "../tools/logger.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace neroshop {

//-----------------------------------------------------------------------------

Socks5Client::Socks5Client(const char* host, uint16_t port, std::shared_ptr<neroshop::TorManager> tm)
        : socks_host_(host), socks_port_(port), sockfd_(-1) 
{
    tor_manager = std::move(tm);
    
    /*auto base_path = get_default_tor_path(); // ~/.config/neroshop/tor
    auto keys_path = base_path / TOR_HIDDEN_SERVICE_DIR_FOLDER_NAME; // ~/.config/neroshop/tor/hidden_service
    auto last_onion_file = keys_path / "last_onion_address.txt"; // ~/.config/neroshop/tor/hidden_service/last_onion_address.txt
    
    if(tor_manager && tor_manager->is_tor_ready()) {
        neroshop::log_debug("Socks5Client: Got new onion address: {}", tor_manager->get_onion_address());
        // Save onion address string to "~/.config/neroshop/tor/last_onion_address.txt" so we can load it later
        std::ofstream outfile(last_onion_file);
        if (outfile.is_open()) {
            outfile << tor_manager->get_onion_address() << std::endl;
            outfile.close();
            neroshop::log_debug("Socks5Client: Saved new onion address to {}", last_onion_file.string());
        } else {
            neroshop::log_error("Socks5Client: Error opening {} for writing", last_onion_file.string());
            // Consider throwing an exception or handling the error appropriately
        }

        // Save assigned port
        this->port_ = reserve_available_port(TOR_HIDDEN_SERVICE_PORT);
    }*/
    // Save assigned port
    this->port_ = reserve_available_port(TOR_HIDDEN_SERVICE_PORT);
}
        
//-----------------------------------------------------------------------------

Socks5Client::~Socks5Client() {
    if (sockfd_ != -1) close(sockfd_);
}

//-----------------------------------------------------------------------------

bool Socks5Client::socks5_handshake(const char* dest_host, uint16_t dest_port) {
    // 1. Send greeting (no authentication)
    unsigned char greeting[3] = {0x05, 0x01, 0x00}; // SOCKS5, 1 method, no auth
    if (::send(sockfd_, greeting, 3, 0) != 3) return false;

    // 2. Receive method selection
    unsigned char method_select[2];
    if (::recv(sockfd_, method_select, 2, 0) != 2) return false;
    if (method_select[1] != 0x00) { // 0x00 = no auth
        std::cerr << "SOCKS5 proxy requires unsupported auth method\n";
        return false;
    }

    // 3. Send connection request
    // Format: VER(1) CMD(1) RSV(1) ATYP(1) DST.ADDR DST.PORT
    // CMD=0x01 connect, ATYP=0x03 domain name
    size_t host_len = strlen(dest_host);
    size_t req_len = 7 + host_len;
    unsigned char* req = new unsigned char[req_len];
    req[0] = 0x05;        // VER
    req[1] = 0x01;        // CMD = CONNECT
    req[2] = 0x00;        // RSV
    req[3] = 0x03;        // ATYP = domain name (correct for .onion)
    req[4] = (unsigned char)host_len; // length of domain
    memcpy(req + 5, dest_host, host_len);
    req[5 + host_len] = (dest_port >> 8) & 0xFF; // port high byte
    req[6 + host_len] = dest_port & 0xFF;        // port low byte

    if (::send(sockfd_, req, req_len, 0) != (ssize_t)req_len) {
        delete[] req;
        return false;
    }
    delete[] req;

    // 4. Receive server response
    // Response: VER(1), REP(1), RSV(1), ATYP(1), BND.ADDR, BND.PORT
    unsigned char resp[10];
    ssize_t n = ::recv(sockfd_, resp, sizeof(resp), 0);
    if (n < 7 || resp[1] != 0x00) { // 0x00 = succeeded
        neroshop::log_error("SOCKS5 proxy connection failed, REP={}", (int)resp[1]);
        return false;
    }

    // Connection established through SOCKS5 proxy
    return true;
}

//-----------------------------------------------------------------------------

bool Socks5Client::socks5_handshake_auth(const char* dest_host, uint16_t dest_port) { // untested
    // 1. Send greeting with no-auth and username/password
    unsigned char greeting[4] = {0x05, 0x02, 0x00, 0x02}; // SOCKS5, 2 methods: no auth, username/password
    if (::send(sockfd_, greeting, 4, 0) != 4) return false;

    // 2. Receive method selection
    unsigned char method_select[2];
    if (::recv(sockfd_, method_select, 2, 0) != 2) return false;
    if (method_select[1] == 0x02) {
        // Username/password auth selected
        // Build auth packet
        std::string username = "user";
        std::string password = "pass";
        size_t len = 3 + username.size() + password.size();
        std::vector<unsigned char> auth_packet(len);
        auth_packet[0] = 0x01; // version
        auth_packet[1] = (unsigned char)username.size();
        memcpy(&auth_packet[2], username.data(), username.size());
        auth_packet[2 + username.size()] = (unsigned char)password.size();
        memcpy(&auth_packet[3 + username.size()], password.data(), password.size());

        ::send(sockfd_, auth_packet.data(), auth_packet.size(), 0);

        // Receive auth response
        unsigned char auth_resp[2];
        ::recv(sockfd_, auth_resp, 2, 0);
        if (auth_resp[1] != 0x00) {
            // Auth failed
            return false;
        }
    } else if (method_select[1] != 0x00) {
        // Unsupported auth method
        return false;
    }

    // 3. Send connection request
    // Format: VER(1) CMD(1) RSV(1) ATYP(1) DST.ADDR DST.PORT
    // CMD=0x01 connect, ATYP=0x03 domain name
    size_t host_len = strlen(dest_host);
    size_t req_len = 7 + host_len;
    unsigned char* req = new unsigned char[req_len];
    req[0] = 0x05;        // VER
    req[1] = 0x01;        // CMD = CONNECT
    req[2] = 0x00;        // RSV
    req[3] = 0x03;        // ATYP = domain name
    req[4] = (unsigned char)host_len; // length of domain
    memcpy(req + 5, dest_host, host_len);
    req[5 + host_len] = (dest_port >> 8) & 0xFF; // port high byte
    req[6 + host_len] = dest_port & 0xFF;        // port low byte

    if (::send(sockfd_, req, req_len, 0) != (ssize_t)req_len) {
        delete[] req;
        return false;
    }
    delete[] req;

    // 4. Receive server response
    // Response: VER(1), REP(1), RSV(1), ATYP(1), BND.ADDR, BND.PORT
    unsigned char resp[10];
    ssize_t n = ::recv(sockfd_, resp, sizeof(resp), 0);
    if (n < 7 || resp[1] != 0x00) { // 0x00 = succeeded
        neroshop::log_error("SOCKS5 proxy connection failed, REP={}", (int)resp[1]);
        return false;
    }

    // Connection established through SOCKS5 proxy
    return true;
}

//-----------------------------------------------------------------------------

uint16_t Socks5Client::reserve_available_port(uint16_t preferred_port) {
    if (preferred_port > 65535) {
        throw std::invalid_argument("Invalid port");
    }
    
    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return 0;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(preferred_port);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (::bind(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        // Try fallback
        addr.sin_port = htons(preferred_port + 8);//htons(0);
        if (::bind(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0) {
            close(sockfd);
            return 0;
        }
    }

    socklen_t addrlen = sizeof(addr);
    getsockname(sockfd, (sockaddr*)&addr, &addrlen);
    uint16_t assigned_port = ntohs(addr.sin_port);

    close(sockfd); // We just wanted to test port availability

    return assigned_port;
}

//-----------------------------------------------------------------------------

// Connect to SOCKS5 proxy and then to dest_host:dest_port via Tor
void Socks5Client::connect(const char* dest_host, uint16_t dest_port) {
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) throw std::runtime_error("Failed to create socket");

    sockaddr_in proxy_addr{};
    proxy_addr.sin_family = AF_INET;
    uint16_t socks_port = get_socks_port(); // The actual socks port
    proxy_addr.sin_port = htons(socks_port);
    if (inet_pton(AF_INET, socks_host_, &proxy_addr.sin_addr) <= 0)
        throw std::runtime_error("Invalid SOCKS proxy IP");

    if (::connect(sockfd_, (sockaddr*)&proxy_addr, sizeof(proxy_addr)) < 0) {
        close(sockfd_);
        sockfd_ = -1;
        throw std::runtime_error("Failed to connect to SOCKS proxy");
    }

    // Perform SOCKS5 handshake here:
    if (!socks5_handshake(dest_host, dest_port)) {
        close(sockfd_);
        sockfd_ = -1;
        throw std::runtime_error("SOCKS5 handshake failed");
    }
}
    
//-----------------------------------------------------------------------------

// Send data through Tor SOCKS5 connection
ssize_t Socks5Client::send(const void* buf, size_t len, int flags) {
    if (sockfd_ == -1) throw std::runtime_error("SOCKS5 socket not connected");
    return ::send(sockfd_, buf, len, flags);
}

//-----------------------------------------------------------------------------

// Receive data through Tor SOCKS5 connection
ssize_t Socks5Client::recv(void* buf, size_t len, int flags) {
    if (sockfd_ == -1) throw std::runtime_error("SOCKS5 socket not connected");
    return ::recv(sockfd_, buf, len, flags);
}

//-----------------------------------------------------------------------------

void Socks5Client::adopt_socket(int fd) {
    if (sockfd_ != -1) close(sockfd_);
    sockfd_ = fd;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void Socks5Client::set_tor_manager(std::shared_ptr<neroshop::TorManager> tm) {
    tor_manager = std::move(tm);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int Socks5Client::get_socket() const noexcept { return sockfd_; }

//-----------------------------------------------------------------------------

uint16_t Socks5Client::get_socks_port() const noexcept { 
    return tor_manager ? tor_manager->get_socks_port() : socks_port_;
}

//-----------------------------------------------------------------------------

int Socks5Client::get_tor_progress() const noexcept {
    return tor_manager ? tor_manager->get_bootstrap_progress() : -1;
}

//-----------------------------------------------------------------------------

std::string Socks5Client::get_tor_address() const noexcept {
    return tor_manager ? tor_manager->get_onion_address() : "";
}

//-----------------------------------------------------------------------------

std::shared_ptr<TorManager> Socks5Client::get_tor_manager() const noexcept {
    return tor_manager;
}

//-----------------------------------------------------------------------------

uint16_t Socks5Client::get_port() const noexcept { return port_; }

//-----------------------------------------------------------------------------

} // namespace neroshop
