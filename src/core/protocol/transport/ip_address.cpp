#include "ip_address.hpp"

#include <vector>
#include <array>
#include <sstream>
#include <regex>
#include <chrono>
#include <thread>
#include <fstream>
#include <stdexcept>
#include <string>
#include <cstring>

namespace neroshop {

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

std::string get_public_ip_address() {
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

std::string get_public_ip_address_tor() {
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

std::string get_device_ip_address() {
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

std::string ip::resolve(const std::string& url) {
    addrinfo hints{}, *res;
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP

    if (getaddrinfo(url.c_str(), nullptr, &hints, &res) != 0) {
        return ""; // failed to resolve url
    }

    std::string ip;
    char ipstr[INET6_ADDRSTRLEN];
    void* addr;
    if (res->ai_family == AF_INET) {
        sockaddr_in* ipv4 = (sockaddr_in*)res->ai_addr;
        addr = &(ipv4->sin_addr);
    } else {
        sockaddr_in6* ipv6 = (sockaddr_in6*)res->ai_addr;
        addr = &(ipv6->sin6_addr);
    }

    inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));
    ip = ipstr;

    freeaddrinfo(res);

    return ip;
}

// Function to resolve hostname to IP address
std::vector<std::string> ip::resolve_v2(const std::string& hostname) {
    std::vector<std::string> ips;
    struct addrinfo hints, *res;
    int status;

    // Clear the hints struct
    memset(&hints, 0, sizeof hints);

    // Set the address family to either IPv4 or IPv6
    hints.ai_family = AF_UNSPEC;

    // Set the socket type to either TCP or UDP
    hints.ai_socktype = SOCK_STREAM;

    // Get the address information for the hostname
    if ((status = getaddrinfo(hostname.c_str(), NULL, &hints, &res)) != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return ips;
    }

    // Loop through all the addresses and add them to the vector
    for (struct addrinfo* p = res; p != NULL; p = p->ai_next) {
        void* addr;
        char ip[INET6_ADDRSTRLEN];

        if (p->ai_family == AF_INET) { // IPv4 address
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
            addr = &(ipv4->sin_addr);
        } else { // IPv6 address
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
            addr = &(ipv6->sin6_addr);
        }

        // Convert the IP address to a string and add it to the vector
        inet_ntop(p->ai_family, addr, ip, sizeof ip);
        ips.push_back(ip);
    }

    // Free the address information
    freeaddrinfo(res);

    return ips;
}

// The is_valid_url function assumes that a valid URL starts with a scheme (such as "http" or "https") and contains a host name, so it would consider "router.bittorrent.com" as not a valid URL
bool is_valid_url(const std::string& url) {
  static const std::regex pattern(
      R"(^(https?|ftp):\/\/([A-Za-z0-9\-\.]+)(?::(\d+))?([\/|\?].*)?$)");

  return std::regex_match(url, pattern);
}

bool is_host_reachable(const std::string& hostname) {
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

int get_ip_type(const std::string& address) {
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

bool create_sockaddr(const std::string& address, int port, struct sockaddr_storage& node_addr) {
    memset(&node_addr, 0, sizeof(node_addr));
        
    if(!is_hostname(address)) { // works with url, ipv4 and ipv6 addresses
        std::cerr << "is_hostname: Invalid address\n";
        return false;
    }

    std::string ip_address = ip::resolve(address);
    
    if(is_ipv4(ip_address)) {
        node_addr.ss_family = AF_INET; // Set IPv4 family
        struct sockaddr_in * addr4 = reinterpret_cast<sockaddr_in*>(&node_addr);
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(port); // port goes here
        inet_pton(AF_INET, ip_address.c_str(), &addr4->sin_addr);/*if(inet_pton(AF_INET, address.c_str(), &addr4->sin_addr) == 0) {//; // inet_pton only works with dotted decimal IPv4 address strings
            std::cerr << "Invalid IPv4 address\n";
            return false;
        }*/
        std::cout << "IPv4 address created\n";
    }
    
    else if(is_ipv6(ip_address)) {
        node_addr.ss_family = AF_INET6; // Set IPv6 family
        struct sockaddr_in6 * addr6 = reinterpret_cast<sockaddr_in6*>(&node_addr);
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(port);
        inet_pton(AF_INET6, ip_address.c_str(), &addr6->sin6_addr);/*if(inet_pton(AF_INET6, address.c_str(), &addr6->sin6_addr) == 0) { // //  If it returns 0, it means that the input string was not in a valid format for the specified address family
            std::cerr << "Invalid IPv6 address\n";
            return false;
        }*/
        std::cout << "IPv6 address created\n";
    }
    
    else {
        std::cerr << "create_socketaddr: Invalid address\n";
        return false;
    }
        
    return true;
}

bool is_ipv4(const std::string& address) {
    static const std::regex ipv4_regex("^([0-9]{1,3}\\.){3}[0-9]{1,3}$");
    return std::regex_match(address, ipv4_regex);
}

bool is_ipv6(const std::string& address) {
    static const std::regex ipv6_regex("^(([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}|[0-9a-fA-F]{1,4}(:[0-9a-fA-F]{1,4}){0,6})$");
    return std::regex_match(address, ipv6_regex);
}

bool is_hostname(const std::string& address) {
    addrinfo hints = {};
    hints.ai_flags = AI_CANONNAME;

    addrinfo* result;
    int error = getaddrinfo(address.c_str(), nullptr, &hints, &result);
    if (error) {
        return false;
    }

    freeaddrinfo(result);
    return true;
}

std::tuple<std::string, int> parse_multiaddress(const std::string& multiaddress) {
    std::regex pattern("/ip[46]/((?:\\d{1,3}\\.){3}\\d{1,3}|\\[[0-9a-fA-F:]+\\])/tcp/(\\d+)");

    std::smatch match;
    if (std::regex_match(multiaddress, match, pattern)) {
        if (is_ipv4(match[1])) {
            return std::make_tuple(match[1], std::stoi(match[2]));
        } else if (is_ipv6(match[1])) {
            return std::make_tuple(match[1].str().substr(1, match[1].str().size() - 2), std::stoi(match[2])); // Remove the square brackets from the IPv6 address
        }
    }

    throw std::invalid_argument("Invalid multiaddress format");
    return std::tuple<std::string, int>(); // This creates a new tuple with an empty string ("") and zero integer value (0).
}

bool ip::is_localhost(const char* ip_str) {
    struct in_addr ip_addr;
    if (inet_pton(AF_INET, ip_str, &ip_addr) == 1) {
        // IPv4
        return ip_addr.s_addr == htonl(INADDR_LOOPBACK);
    } else if (inet_pton(AF_INET6, ip_str, &ip_addr) == 1) {
        // IPv6
        struct in6_addr* ipv6_addr = (struct in6_addr*)&ip_addr;
        return memcmp(ipv6_addr, &in6addr_loopback, sizeof(struct in6_addr)) == 0;
    }

    // Invalid IP address
    return false;
}

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

    std::string ipv4_address = "192.168.1.1";
    std::string ipv6_address = "2001:0db8:85a3:0000:0000:8a2e:0370:7334";
    std::string invalid_address = "foo";

    std::cout << ipv4_address << " is IPv4: " << is_ipv4(ipv4_address) << "\n";
    std::cout << ipv6_address << " is IPv6: " << is_ipv6(ipv6_address) << "\n";
    std::cout << invalid_address << " is IPv4: " << is_ipv4(invalid_address) << "\n";
    std::cout << invalid_address << " is IPv6: " << is_ipv6(invalid_address) << "\n";
    
    std::vector<std::string> ips = neroshop::resolve_hostname("www.google.com");
    for (const auto& ip : ips) {
        std::cout << ip << std::endl;
    }    
    
    return 0;
} // g++ ip_address.cpp -o ip -std=c++17
*/
