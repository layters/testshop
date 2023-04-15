#pragma once

#if defined(_WIN32)
#include <winsock2.h> // core header for Winsock2
#include <ws2tcpip.h> // header for TCP/IP protocols
#include <iphlpapi.h> // header for IP helper functions
#endif

#if defined(__gnu_linux__)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
//#include <fcntl.h>
//#include <ifaddrs.h>
//#include <errno.h>
//#include <poll.h>
#endif

#include <iostream>
#include <string>

namespace neroshop {
    std::string get_public_ip_address();
    std::string get_public_ip_address_tor(); // tor is required but this always fails so I dunno ...
    
    std::string get_device_ip_address();
    
    std::string url_to_ip(const std::string& url);
    
    bool is_valid_url(const std::string& url);
    
    bool is_host_reachable(const std::string& hostname); // untested
    
    int get_ip_type(const std::string& ip_address);
    
    bool create_sockaddr(const std::string& address, int port, struct sockaddr_storage& node_addr);
}
