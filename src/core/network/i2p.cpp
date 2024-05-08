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

bool neroshop::is_i2pd_running() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "Error: Socket creation failed" << std::endl;
        return false;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(7656); // use i2p's SAMv3 TCP port since UDP does not listen for connections (7655=UDP;7656=TCP) 
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
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
}

static int i2pd_sam_test() {
    // Address and ports for SAM bridge
    std::string address = "127.0.0.1";
    int portTCP = 7656;
    int portUDP = 7655;

    // Create SAM bridge
    i2p::client::SAMBridge sam_bridge(address, 0/*portTCP*/, portUDP, false); // Set the last parameter to true for single-threaded operation

    // Start the SAM bridge
    sam_bridge.Start();

    // Create a SAM session
    std::string session_id = "mysession";
    i2p::client::SAMSessionType session_type = i2p::client::eSAMSessionTypeDatagram;//i2p::client::eSAMSessionTypeStream;
    std::string destination = ""; // empty string means transient
    std::map<std::string, std::string> params; // additional parameters if needed

    std::shared_ptr<i2p::client::SAMSession> session = sam_bridge.CreateSession(session_id, session_type, destination, &params);

    // Add the session to the bridge
    sam_bridge.AddSession(session);

    // Perform SAM-related operations
    // ...

    // Stop the SAM bridge
    sam_bridge.Stop();
    
    return 0;
}
/*int main(int argc, char* argv[]) {
    int error = i2pd_sam_test();

    return error;
}*/

// g++ i2p.cpp -o i2p -I../../../external/i2pd/daemon -I../../../external/i2pd/i18n -I../../../external/i2pd/libi2pd -I../../../external/i2pd/libi2pd_client -L../../../build -li2pdclient -li2pd -li2pdlang -lboost_program_options -lboost_filesystem -lboost_system -lboost_date_time -lcrypto -lssl -lz -lpthread 

// g++ i2p.cpp -o i2p -I../../../external/i2psam -L../../../build -li2psam
