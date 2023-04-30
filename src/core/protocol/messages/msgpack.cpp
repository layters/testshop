#include "msgpack.hpp"

#define JSON_USE_MSGPACK
#include <nlohmann/json.hpp>

#include "../p2p/kademlia.hpp"

bool neroshop::msgpack::send_data(int sockfd, const std::vector<uint8_t>& packed) {
    if(sockfd < 0) throw std::runtime_error("socket is dead");

    /*nlohmann::json j = {{"foo", "bar"}, {"baz", 1}};
    std::vector<uint8_t> packed = nlohmann::json::to_msgpack(j);*/

    // Send the packed data using write()
    ssize_t sent_bytes = ::send(sockfd, packed.data(), packed.size(), 0);//sendto(sockfd, packed.data(), packed.size(), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (sent_bytes == -1) {
        perror("write");
        return false;
    }    

    return true;
}

std::string neroshop::msgpack::receive_data(int sockfd) {
    if(sockfd < 0) throw std::runtime_error("socket is dead");
        
    const int BUFFER_SIZE = 4096;

    // Receive the packed data using recvfrom()
    std::vector<uint8_t> buffer(BUFFER_SIZE);
    std::string json_str;
    ssize_t total_recv_bytes = 0;
    ssize_t recv_bytes;
    do {
        recv_bytes = ::recv(sockfd, buffer.data() + total_recv_bytes, BUFFER_SIZE - total_recv_bytes, 0);
        if (recv_bytes == -1) {
            perror("read");//perror("recv");
            return "";
        }
        total_recv_bytes += recv_bytes;
    } while (recv_bytes > 0 && total_recv_bytes < BUFFER_SIZE);

    // Convert the packed data to JSON string
    try {
        nlohmann::json j = nlohmann::json::from_msgpack(buffer.data(), total_recv_bytes);
        json_str = j.dump();
        std::cout << "Request received: " << json_str << std::endl;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Error parsing message: " << e.what() << std::endl;
    }
    
    return json_str;

}
