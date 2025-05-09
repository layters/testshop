#include "i2p.hpp"

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#endif

#if defined(__gnu_linux__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#endif
#include <iostream>

namespace neroshop {

bool is_i2p_running() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "Error: Socket creation failed" << std::endl;
        return false;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(7656); // use i2p's SAMv3 TCP port since UDP does not listen for connections (7655=UDP;7656=TCP) 
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (::connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        if (errno == EADDRINUSE) {
            close(sockfd);
            return true;
        } else {
            std::cerr << "Failed to connect to i2p's SAMv3 TCP port: " << strerror(errno) << std::endl;
            close(sockfd);
            return false;
        }
    } else {
        close(sockfd);
        return true;
    }
    
    return false;
}

}

/*int main(int argc, char* argv[]) {
    int running = is_i2p_running();
    std::cout << "I2P running: " << ((running) ? "yes" : "no") << "\n";
    
    return 0;
}*/

// g++ i2p.cpp -o i2p -I../../../external/i2pd/daemon -I../../../external/i2pd/i18n -I../../../external/i2pd/libi2pd -I../../../external/i2pd/libi2pd_client -L../../../build -li2pdclient -li2pd -li2pdlang -lboost_program_options -lboost_filesystem -lboost_system -lboost_date_time -lcrypto -lssl -lz -lpthread 

// g++ i2p.cpp -o i2p -I../../../external/i2psam -L../../../build -li2psam
