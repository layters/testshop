#include "../core/protocol/transport/zmq_client.hpp"
#include "../core/protocol/transport/zmq_server.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <thread>

#include "../core/protocol/rpc/json_rpc.hpp"
#include "../core/protocol/messages/msgpack.hpp"

#include <nlohmann/json.hpp>

#include <iostream>

std::string extract_json_payload(const std::string& request) {
    // Find the start of the JSON payload
    const std::string json_start = "\r\n\r\n";
    const auto json_pos = request.find(json_start);
    if (json_pos == std::string::npos) {
        // JSON payload not found
        return "";
    }
    // Extract the JSON payload
    const auto json_payload = request.substr(json_pos + json_start.size());
    return json_payload;
}

//-----------------------------------------------------------------------------

//void rpc_server() {}

//-----------------------------------------------------------------------------

void ipc_server() {
    try {
        // create ZmqServer instance
        neroshop::ZmqServer server("tcp://*:5555", false);
        
        while (true) {
            std::vector<uint8_t> request;//std::string request;
            // wait for incoming message from client
            server.receive(request);
            nlohmann::json j = nlohmann::json::from_msgpack(request);
            std::cout << "Received request: " << j.dump() << std::endl;//std::cout << "Received request: " << request << std::endl;
            
            // process JSON request and generate response
            std::string message = j["message"];
            nlohmann::json response_json = {
                {"message", "Response to " + message}
            };//std::string response = "Response to " + request;
            // serialize response JSON object to binary
            std::vector<uint8_t> response = nlohmann::json::to_msgpack(response_json);
            
            // send response to client
            server.send(response);
            std::cout << "Sent response: " << response_json.dump() << std::endl;//std::cout << "Sent response: " << response << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "Error occurred: " << e.what() << std::endl;
        return;
    }
}

//-----------------------------------------------------------------------------

void dht_server() {
}

//-----------------------------------------------------------------------------

int main()
{
    // Create threads for server(s)
    std::thread ipc_thread(ipc_server); // For IPC communication between the local GUI client and the local daemon server
    std::thread dht_thread(dht_server); // DHT communication for peer discovery and data storage
    //std::thread rpc_thread(rpc_server); // RPC communication for processing requests from outside clients (can be disabled)

    // Wait for threads to finish
    ipc_thread.join();
    dht_thread.join();
    //rpc_thread.join();

    return 0;
} // g++ -D NEROSHOP_USE_LIBZMQ zmq_client.cpp zmq_server.cpp main.cpp -lzmq
