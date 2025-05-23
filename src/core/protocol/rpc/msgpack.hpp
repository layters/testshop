#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint> // uint8_t

namespace neroshop {

class Node; // forward declaration

namespace msgpack {
    std::vector<uint8_t> process(const std::vector<uint8_t>& request, Node& node, bool ipc_mode = false); // ipc_mode is for when the IPC client (A.K.A local GUI client) makes send_put and send_get requests in real-time

    std::string generate_transaction_id();
    //std::string generate_token(const std::string& node_id, const std::string& info_hash, const std::string& secret);
    //std::string generate_secret(int length);
}

}
