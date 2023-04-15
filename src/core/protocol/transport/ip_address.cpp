#include "ip_address.hpp"

#include <vector>
#include <array>
#include <sstream>
#include <regex>
#include <chrono>
#include <thread>
#include <fstream>
#include <stdexcept>

std::vector<std::string> IP_SOURCES = {
    "http://httpbin.org/ip", 
    "https://api.ip.sb/ip", 
    "https://api.ipify.org", 
    "https://icanhazip.com/", 
    "https://ident.me/", 
    "https://ifconfig.co/ip", 
    "https://ifconfig.me/ip", 
    "https://ipecho.net/plain", 
    "https://ipapi.co/ip", 
    "https://ipinfo.io/ip",
    "https://ip.seeip.org/",
    "https://checkip.amazonaws.com/"
};

std::string neroshop::get_public_ip_address() {
    struct addrinfo hints, *res;
    int sockfd;
    std::string ip_address;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    for (const auto& source : IP_SOURCES) {
        std::regex url_regex("^(https?://)?([^/]+)");
        std::smatch url_match;

        if (!std::regex_search(source, url_match, url_regex)) {
            continue;
        }

        const std::string& host = url_match[2];
        if (getaddrinfo(host.c_str(), "http", &hints, &res) != 0) {
            std::cerr << "Error resolving hostname: " << host << std::endl;
            continue;
        }

        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd < 0) {
            perror("Error creating socket");
            continue;
        }

        if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("Error connecting to server");
            continue;
        }

        char buffer[1024] = {0};
        if (send(sockfd, ("GET / HTTP/1.1\r\nHost: " + host + "\r\n\r\n").c_str(), 32 + host.size(), 0) < 0) {
            perror("Error sending data to server");
            continue;
        }

        if (recv(sockfd, buffer, 1024, 0) < 0) {
            perror("Error receiving data from server");
            continue;
        }

        close(sockfd);
        std::string response(buffer);
        std::regex ip_regex("(\\d{1,3}\\.){3}\\d{1,3}");
        std::smatch ip_match;

        if (std::regex_search(response, ip_match, ip_regex)) {
            ip_address = ip_match[0];
            break;
        }
    }

    if (ip_address.empty()) {
        throw std::runtime_error("Unable to retrieve public IP address");
    }

    return ip_address;
}

std::string neroshop::get_public_ip_address_tor() {
    struct addrinfo hints, *res;
    int sockfd;
    std::string ip_address;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    for (const auto& source : IP_SOURCES) {
        std::regex url_regex("^(https?://)?([^/]+)");
        std::smatch url_match;

        if (!std::regex_search(source, url_match, url_regex)) {
            continue;
        }

        const std::string& host = url_match[2];
        if (getaddrinfo(host.c_str(), "http", &hints, &res) != 0) {
            std::cerr << "Error resolving hostname: " << host << std::endl;
            continue;
        }

        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd < 0) {
            perror("Error creating socket");
            continue;
        }

        struct sockaddr_in* tor_addr = (struct sockaddr_in*) res->ai_addr;
        tor_addr->sin_port = htons(9050);
        tor_addr->sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
            perror("Error connecting to Tor");
            continue;
        }

        const char* request = ("GET / HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n").c_str();
        size_t request_len = strlen(request);
        if (send(sockfd, request, request_len, 0) < 0) {
            perror("Error sending data to server");
            continue;
        }

        char buffer[1024] = {0};
        if (recv(sockfd, buffer, 1024, 0) < 0) {
            perror("Error receiving data from server");
            continue;
        }

        close(sockfd);
        std::string response(buffer);
        std::regex ip_regex("(\\d{1,3}\\.){3}\\d{1,3}");
        std::smatch ip_match;

        if (std::regex_search(response, ip_match, ip_regex)) {
            ip_address = ip_match[0];
            break;
        }
    }

    if (ip_address.empty()) {
        throw std::runtime_error("Unable to retrieve public IP address via Tor");
    }

    return ip_address;
}

std::string neroshop::get_device_ip_address() {
    int sockfd;
    char buffer[1024];
    struct sockaddr_in serv_addr;
    socklen_t len;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("8.8.8.8");
    serv_addr.sin_port = htons(53);

    connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    len = sizeof(serv_addr);
    getsockname(sockfd, (struct sockaddr *)&serv_addr, &len);

    return inet_ntoa(serv_addr.sin_addr);
}

std::string neroshop::url_to_ip(const std::string& url) {
    addrinfo hints{}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(url.c_str(), nullptr, &hints, &res) != 0) {
        return ""; // failed to resolve url
    }

    sockaddr_in* ipv4 = (sockaddr_in*)res->ai_addr;
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ipv4->sin_addr), ip, INET_ADDRSTRLEN);

    freeaddrinfo(res);

    return ip;
}

// The is_valid_url function assumes that a valid URL starts with a scheme (such as "http" or "https") and contains a host name, so it would consider "router.bittorrent.com" as not a valid URL
bool neroshop::is_valid_url(const std::string& url) {
  static const std::regex pattern(
      R"(^(https?|ftp):\/\/([A-Za-z0-9\-\.]+)(?::(\d+))?([\/|\?].*)?$)");

  return std::regex_match(url, pattern);
}

bool neroshop::is_host_reachable(const std::string& hostname) {
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* res;
    int ret = getaddrinfo(hostname.c_str(), nullptr, &hints, &res);
    if (ret != 0) {
        std::cerr << "getaddrinfo failed: " << gai_strerror(ret) << '\n';
        return false;
    }

    bool reachable = false;
    for (auto* p = res; p != nullptr; p = p->ai_next) {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in*>(p->ai_addr)->sin_addr, ip, sizeof(ip));
        std::cout << "Resolved " << hostname << " to " << ip << '\n';
        reachable = true;
    }

    freeaddrinfo(res);

    return reachable;
}

int neroshop::get_ip_type(const std::string& address) {
    // Use getaddrinfo to resolve the address to a sockaddr_storage structure that can hold both IPv4 and IPv6 addresses
    // Determine the address family of the IP address
    int addr_family = AF_UNSPEC;
    if (strchr(address.c_str(), ':')) {
        addr_family = AF_INET6;
    } else {
        addr_family = AF_INET;
    }
    return addr_family;
}

bool neroshop::create_sockaddr(const std::string& address, int port, struct sockaddr_storage& node_addr) {
    // No need to use `url_to_ip()` since `inet_pton()` handles that
    
    int ip_type = get_ip_type(address);
    //std::cout << "ip type: " << ip_type << " " << ((ip_type == AF_INET6) ? "IPv6" : "IPv4") << "\n";

    memset(&node_addr, 0, sizeof(node_addr));
    
    if (ip_type == AF_INET) {
        struct sockaddr_in * addr4 = reinterpret_cast<sockaddr_in*>(&node_addr);
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(port); // port goes here
        if(inet_pton(AF_INET, address.c_str(), &addr4->sin_addr) == 0) {//; // IP address goes here // inet_pton (short for "presentation to network") is a more modern and versatile function than inet_addr and can handle both IPv4 and IPv6 addresses, making it a more flexible and future-proof option
            // address is not a valid IPv4 address, try resolving it as a hostname - causes error for some reason
            /*struct addrinfo hints, *res;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            if (getaddrinfo(address.c_str(), NULL, &hints, &res) != 0) {
                std::cerr << "Failed to resolve hostname\n";
                return false;
            }
            memcpy(&addr4->sin_addr, &((struct sockaddr_in*)res->ai_addr)->sin_addr, sizeof(struct in_addr));
            freeaddrinfo(res);*/
        }
    } else if (ip_type == AF_INET6) {
        struct sockaddr_in6 * addr6 = reinterpret_cast<sockaddr_in6*>(&node_addr);
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(port);
        if(inet_pton(AF_INET6, address.c_str(), &addr6->sin6_addr) == 0) {//;
            // address is not a valid IPv6 address, try resolving it as a hostname - causes error for some reason
            /*struct addrinfo hints, *res;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET6;
            hints.ai_socktype = SOCK_STREAM;
            if (getaddrinfo(address.c_str(), NULL, &hints, &res) != 0) {
                std::cerr << "Failed to resolve hostname\n";
                return false;
            }
            memcpy(&addr6->sin6_addr, &((struct sockaddr_in6*)res->ai_addr)->sin6_addr, sizeof(struct in6_addr));
            freeaddrinfo(res);*/
        }
    } else {
        std::cerr << "Invalid address" << std::endl;
        return false;
    }
    return true;
}

/*int main() {
    try {
        std::string device_ip_address = neroshop::get_device_ip_address();
        std::cout << "Device IP address: " << device_ip_address << "\n";
        std::string public_ip_address = neroshop::get_public_ip_address();
        std::cout << "Public IP address: " << public_ip_address << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    std::string url = "http://example.com";
    if (neroshop::is_valid_url(url)) {
        // URL is valid
        // Use it to make a connection or something else
    } else {
        // URL is not valid
        // Handle the error
    }
    
    if (!neroshop::is_host_reachable("router.bittorrent.com")) {
    std::cerr << "router.bittorrent.com is not reachable\n";
    } else {
        // use "router.bittorrent.com" as an address
    }

    const char *address = "127.0.0.1";
    int ip_type = get_ip_type(address);
    if (ip_type == AF_INET) {
        // Handle IPv4 address
    } else if (ip_type == AF_INET6) {
        // Handle IPv6 address
    } else {
        // Handle error
    }

    return 0;
} // g++ ip_address.cpp -o ip -std=c++17
*/
