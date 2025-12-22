#include "socks5_client.hpp"

#include "tor_manager.hpp"
#include "../tools/logger.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>      // fcntl, F_GETFL, F_SETFL, O_NONBLOCK
#include <poll.h>       // poll, pollfd, POLLOUT
#include <errno.h>      // errno
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace neroshop {

//-----------------------------------------------------------------------------

Socks5Client::Socks5Client(const char* host, uint16_t port, std::shared_ptr<neroshop::TorManager> tm)
        : socks_host_(host), socks_port_(port), tor_manager(std::move(tm)) 
{
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
    close();
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

uint16_t Socks5Client::reserve_available_port(uint16_t start_port) {
    if (start_port > 65535) throw std::invalid_argument("Invalid port");
    
    // Test range: 50882-50889
    for (uint16_t port = start_port; port <= start_port + 7; ++port) {
        int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) continue;
        
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");
        
        if (::bind(sockfd, (sockaddr*)&addr, sizeof(addr)) == 0) {
            socklen_t addrlen = sizeof(addr);
            getsockname(sockfd, (sockaddr*)&addr, &addrlen);
            uint16_t assigned_port = ntohs(addr.sin_port);
            ::close(sockfd);
            return assigned_port;  // Found available port
        }
        ::close(sockfd);
    }
    
    return 0;  // No ports available
}

//-----------------------------------------------------------------------------

// Connect to SOCKS5 proxy and then to dest_host:dest_port via Tor
void Socks5Client::connect(const char* dest_host, uint16_t dest_port) {
    if (strlen(dest_host) > MAX_DOMAIN_LEN) {
        throw std::runtime_error("Destination host too long");
    }
    if (neroshop::string_tools::ends_with(dest_host, ".onion") && 
        strlen(dest_host) != 62) {  // 56 + ".onion"
        throw std::runtime_error("Invalid .onion address length");
    }

    close();  // Clean slate
    
    sockfd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) throw std::runtime_error("socket() failed");

    // Non-blocking connect for timeout
    int flags = fcntl(sockfd_, F_GETFL, 0);
    fcntl(sockfd_, F_SETFL, flags | O_NONBLOCK);

    sockaddr_in proxy_addr{};
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(get_socks_port());
    if (inet_pton(AF_INET, socks_host_.c_str(), &proxy_addr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid SOCKS proxy IP: " + socks_host_);
    }

    if (::connect(sockfd_, (sockaddr*)&proxy_addr, sizeof(proxy_addr)) < 0) {
        if (errno != EINPROGRESS) {
            throw std::runtime_error("connect() failed: " + std::string(strerror(errno)));
        }
        
        // Poll for connect completion
        pollfd pfd = {sockfd_, POLLOUT, 0};
        int rv = poll(&pfd, 1, CONNECT_TIMEOUT_MS);
        if (rv <= 0) {
            throw std::runtime_error("Connect timeout or error");
        }
        
        int err;
        socklen_t len = sizeof(err);
        getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, &err, &len);
        if (err != 0) {
            throw std::runtime_error("Connect failed: " + std::string(strerror(err)));
        }
    }

    // Back to blocking
    fcntl(sockfd_, F_SETFL, flags);
    set_socket_timeout(IO_TIMEOUT_MS);

    // === PHASE 1: Greeting ===
    unsigned char greeting[3] = {0x05, 0x01, 0x00};
    if (send_all(greeting, 3, 0, IO_TIMEOUT_MS) != 3) {
        throw std::runtime_error("SOCKS5 greeting send failed");
    }

    unsigned char method_reply[2];
    if (recv_all(method_reply, 2, 0, IO_TIMEOUT_MS) != 2 || method_reply[1] != 0x00) {
        throw std::runtime_error("SOCKS5 auth method rejected: " + 
                                std::string(1, method_reply[1]));
    }

    // === PHASE 2: CONNECT request ===
    size_t host_len = strlen(dest_host);
    size_t req_len = 7 + host_len;
    std::vector<unsigned char> req(req_len);
    req[0] = 0x05; req[1] = 0x01; req[2] = 0x00; req[3] = 0x03;
    req[4] = static_cast<unsigned char>(host_len);
    std::memcpy(req.data() + 5, dest_host, host_len);
    uint16_t net_port = htons(dest_port);
    std::memcpy(req.data() + 5 + host_len, &net_port, 2);

    if (send_all(req.data(), req_len, 0, IO_TIMEOUT_MS) != static_cast<ssize_t>(req_len)) {
        throw std::runtime_error("SOCKS5 connect request send failed");
    }

    // === PHASE 3: CONNECT reply (dynamic length) ===
    unsigned char header[4];
    if (recv_all(header, 4, 0, IO_TIMEOUT_MS) != 4 || header[0] != 0x05 || header[1] != 0x00) {
        throw std::runtime_error("SOCKS5 connect failed: " + socks5_error(header[1]));
    }

    size_t remaining_bytes = 0;
    switch (header[3]) {  // ATYP
        case 0x01: remaining_bytes = 6; break;  // IPv4 + port
        case 0x03: {
            unsigned char len_byte[1];
            recv_all(len_byte, 1, 0, IO_TIMEOUT_MS);
            remaining_bytes = 2 + len_byte[0];  // port + domain
            break;
        }
        case 0x04: remaining_bytes = 18; break;  // IPv6 + port
        default: throw std::runtime_error("Unsupported ATYP: " + std::to_string(header[3]));
    }
    
    std::vector<unsigned char> bind_addr(remaining_bytes);
    recv_all(bind_addr.data(), remaining_bytes, 0, IO_TIMEOUT_MS);

    connected_.store(true);
    log_info("SOCKS5 connected to {}:{}", dest_host, dest_port);
}
    
//-----------------------------------------------------------------------------

// Send data through Tor SOCKS5 connection
ssize_t Socks5Client::send(const void* buf, size_t len, int flags) {
    if (!connected_.load()) throw std::runtime_error("Not connected");
    ssize_t rv = send_all(buf, len, flags, IO_TIMEOUT_MS);
    if (rv < 0 && (errno == EPIPE || errno == ECONNRESET)) {
        connected_.store(false);
        throw std::runtime_error("Connection closed by peer");
    }
    return rv;
}

//-----------------------------------------------------------------------------

ssize_t Socks5Client::send_all(const void* buf, size_t len, int flags, int timeout_ms) {
    if (sockfd_ == -1) return -1;
    
    size_t sent = 0;
    const char* ptr = static_cast<const char*>(buf);
    
    while (sent < len) {
        set_socket_timeout(timeout_ms);
        ssize_t rv = ::send(sockfd_, ptr + sent, len - sent, flags);
        if (rv <= 0) return rv;
        sent += rv;
    }
    return sent;
}

//-----------------------------------------------------------------------------

// Receive data through Tor SOCKS5 connection
ssize_t Socks5Client::recv(void* buf, size_t len, int flags) {
    if (!connected_.load()) throw std::runtime_error("Not connected");
    ssize_t rv = recv_all(buf, len, flags, IO_TIMEOUT_MS);
    if (rv <= 0 && (errno == 0 || errno == EAGAIN)) {
        connected_.store(false);
        throw std::runtime_error("Connection timeout/closed");
    }
    return rv;
}

//-----------------------------------------------------------------------------

ssize_t Socks5Client::recv_all(void* buf, size_t len, int flags, int timeout_ms) {
    if (sockfd_ == -1) return -1;
    
    size_t received = 0;
    char* ptr = static_cast<char*>(buf);
    
    while (received < len) {
        set_socket_timeout(timeout_ms);
        ssize_t rv = ::recv(sockfd_, ptr + received, len - received, flags);
        if (rv <= 0) return rv;
        received += rv;
    }
    return received;
}

//-----------------------------------------------------------------------------

void Socks5Client::adopt_socket(int fd) {
    if (sockfd_ != -1) ::close(sockfd_);
    sockfd_ = fd;
}

//-----------------------------------------------------------------------------

void Socks5Client::close() {
    if (sockfd_ != -1) { 
        ::close(sockfd_);
        sockfd_ = -1;
        connected_.store(false);
    }
}

//-----------------------------------------------------------------------------

std::string Socks5Client::socks5_error(uint8_t rep_code) {
    switch (rep_code) {
        case 0x01: return "general SOCKS server failure";
        case 0x02: return "connection not permitted";
        case 0x03: return "network unreachable";
        case 0x04: return "host unreachable";
        case 0x05: return "connection refused";
        case 0x06: return "TTL expired";
        case 0x07: return "command not supported";
        case 0x08: return "address type not supported";
        default: return "unknown error 0x" + std::to_string(rep_code);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool Socks5Client::set_socket_timeout(int timeout_ms) {
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    return setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == 0 &&
           setsockopt(sockfd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == 0;
}

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

bool Socks5Client::is_connected() const {
    return connected_.load();
}

//-----------------------------------------------------------------------------

} // namespace neroshop
