#include "msgpack.hpp"

#define JSON_USE_MSGPACK
#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include "../../version.hpp"
#include "../../tools/logger.hpp"
#include "../p2p/kademlia.hpp"


std::vector<uint8_t> neroshop::msgpack::process(const std::vector<uint8_t>& request, Node& node, bool ipc_mode) {
    nlohmann::json request_object;
    
    nlohmann::json response_object;
    std::vector<uint8_t> response; // bytes

    // Process (parse) the request
    try {
        request_object = nlohmann::json::from_msgpack(request);
        std::cout << "\033[33m" << request_object.dump() << "\033[0m" << std::endl;
    }
    catch(nlohmann::json::parse_error& exception) {
        neroshop::print("Error parsing client request", 1);
        response_object["version"] = std::string(NEROSHOP_DHT_VERSION);//"0.1.0"; // neroshop version
        response_object["error"]["code"] = static_cast<int>(KadResultCode::ParseError); // "code" MUST be an integer
        response_object["error"]["message"] = "Parse error";
        response_object["error"]["data"] = exception.what(); // A Primitive (non-object) or Structured (array) value which may be omitted
        response_object["tid"] = nullptr;
        response = nlohmann::json::to_msgpack(response_object);
        #ifdef NEROSHOP_DEBUG//0
        std::cout << "Response output:\n\033[91m" << response_object.dump(4) << "\033[0m\n";
        #endif
        return response;//return response_object.dump(4);
    }
    //-----------------------------------------------------
    assert(request_object.is_object());
    assert(request_object["version"].is_string());
    std::string neroshop_version = request_object["version"];
    assert(neroshop_version == std::string(NEROSHOP_DHT_VERSION));
    assert(request_object["query"].is_string());
    std::string method = request_object["query"];
    // "args" must contain the querying node's ID 
    assert(request_object["args"].is_object());
    auto params_object = request_object["args"];
    if(!ipc_mode) assert(params_object["id"].is_string()); // querying node's id
    std::string requester_node_id = (ipc_mode) ? node.get_id() : params_object["id"].get<std::string>();
    
    if(!request_object.contains("tid") && !ipc_mode) {
        std::cout << "No tid found, hence a notification that will not receive a response from the server\n";
        return {};
    }
    auto tid = (ipc_mode) ? nullptr : request_object["tid"];
    int code = 0;
    //-----------------------------------------------------
    if(method == "ping") {
        response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
        response_object["response"]["id"] = node.get_id();
    }
    //-----------------------------------------------------
    if(method == "find_node") {
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["target"].is_string()); // target node id sought after by the querying node
        std::string target = params_object["target"];
        
        response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
        response_object["response"]["id"] = node.get_id();
        std::vector<Node*> nodes = node.find_node(target, NEROSHOP_DHT_MAX_CLOSEST_NODES);
        if(nodes.empty()) {
            response_object["response"]["nodes"] = nlohmann::json::array();
        } else {
            std::vector<nlohmann::json> nodes_array;
            for (const auto& n : nodes) {
                nlohmann::json node_object = {
                    {"ip_address", n->get_ip_address()},
                    {"port", n->get_port()}
                };
                nodes_array.push_back(node_object);
            }
            response_object["response"]["nodes"] = nodes_array;
        }
    }
    //-----------------------------------------------------
    if(method == "get_peers") {
        std::cout << "message type is a get_peers\n"; // 
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["info_hash"].is_string()); // info hash
        std::string info_hash = params_object["info_hash"];
        
        response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
        response_object["response"]["id"] = node.get_id();
        // Check if the queried node has peers for the requested infohash
        std::vector<Peer> peers = node.get_peers(info_hash);
        if(peers.empty()) {
            // If the queried node has no peers for the requested infohash,
            // return the K closest nodes in the routing table to the requested infohash
            std::vector<Node*> closest_nodes = node.find_node(info_hash, NEROSHOP_DHT_MAX_CLOSEST_NODES);
            std::vector<nlohmann::json> nodes_array;
            for (const auto& n : closest_nodes) {
                nlohmann::json node_object = {
                    {"id", n->get_id()},
                    {"ip_address", n->get_ip_address()},
                    {"port", n->get_port()}
                };
                nodes_array.push_back(node_object);
                //std::cout << "Node ID: " << n->get_id() << ", Node IP address: " << n->get_ip_address() << ", Node port: " << n->get_port() << std::endl;
            }
            response_object["response"]["nodes"] = nodes_array; // If the queried node has no peers for the infohash, a key "nodes" is returned containing the K nodes in the queried nodes routing table closest to the infohash supplied in the query
        } else {
            std::vector<nlohmann::json> peers_array;
            for (const auto& p : peers) {
                nlohmann::json peer_object = {
                    {"ip_address", p.address},
                    {"port", p.port}
                };
                peers_array.push_back(peer_object);
                //std::cout << "Peer IP address: " << p.address << ", Peer port: " << p.port << std::endl;
            }
            response_object["response"]["values"] = peers_array; // If the queried node has peers for the infohash, they are returned in a key "values" as a list of strings. Each string containing "compact" format peer information for a single peer
        }
        // Generate and include a token value in the response
        auto secret = generate_secret(16);
        std::string token = generate_token(node.get_id(), info_hash, secret); // The reason for concatenating the node ID and info hash is to ensure that the generated token is unique and specific to the peer making the request. This helps prevent replay attacks, where an attacker intercepts and reuses a token generated for another peer.
        response_object["response"]["token"] = token;
    }
    //-----------------------------------------------------
    if(method == "announce_peer") {
        std::cout << "message type is a announce_peer\n";
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["info_hash"].is_string()); // info hash
        std::string info_hash = params_object["info_hash"];
        assert(params_object["token"].is_string());
        std::string token = params_object["token"];
        assert(params_object["port"].is_number_integer());
        int port = params_object["port"];
        
        // Verify the token
        std::string secret = generate_secret(16);
        std::string expected_token = generate_token(node.get_id(), info_hash, secret);
        if (token != expected_token) {
            // Invalid token, return error response
            code = static_cast<int>(KadResultCode::InvalidToken);
            response_object["error"]["code"] = code;
            response_object["error"]["message"] = "Invalid token";
            response_object["tid"] = tid;
            response = nlohmann::json::to_msgpack(response_object);
            return response;
        }

        // Add the peer to the info_hash_peers unordered_map
        node.add_peer(info_hash, {/*ip_address*/"", port});
        
        // Return success response
        response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
        response_object["response"]["id"] = node.get_id();
        response_object["response"]["code"] = code;
        response_object["response"]["message"] = "Peer announced"; // not needed
    }
    //-----------------------------------------------------
    if(method == "get") {
        if(ipc_mode == false) { // For Processing Get Requests from Other Nodes:
            assert(request_object["args"].is_object());
            auto params_object = request_object["args"];
            assert(params_object["key"].is_string());
            std::string key = params_object["key"];

            // Look up the value in the node's own hash table
            std::string value = node.find_value(key);
                    
            if (!value.empty()) {
                // Key found, return success response with value
                response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
                response_object["response"]["id"] = node.get_id();
                response_object["response"]["value"] = value;
            } else {
                // If node does not have the key, check the closest nodes to see if they have it
                std::vector<Node*> closest_nodes = node.find_node(key, NEROSHOP_DHT_MAX_CLOSEST_NODES);

                std::random_device rd;
                std::mt19937 rng(rd());
                std::shuffle(closest_nodes.begin(), closest_nodes.end(), rng);

                std::string closest_node_value;
                for (auto const& closest_node : closest_nodes) {
                    closest_node_value = closest_node->send_get(key);
                    if (!closest_node_value.empty()) {
                        break;
                    }
                }

                if (!closest_node_value.empty()) {
                    response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
                    response_object["response"]["id"] = node.get_id();
                    response_object["response"]["value"] = closest_node_value;
                } else {
                    // Key not found, return error response
                    code = static_cast<int>(KadResultCode::RetrieveFailed);
                    response_object["error"]["code"] = code;
                    response_object["error"]["message"] = "Key not found";
                    response_object["tid"] = tid;
                    response = nlohmann::json::to_msgpack(response_object);
                    return response;
                }
            }

        } else { // For Sending Get Requests to Other Nodes
            assert(request_object["args"].is_object());
            auto params_object = request_object["args"];
            assert(params_object["key"].is_string());
            std::string key = params_object["key"];
                        
            // Send get messages to the closest nodes in your routing table (IPC mode)
            std::string value = node.send_get(key);
            
            // Key not found, return error response
            if (value.empty()) {
                code = static_cast<int>(KadResultCode::RetrieveFailed);
                response_object["error"]["code"] = code;
                response_object["error"]["message"] = "Key not found";
                response_object["tid"] = tid;
                response = nlohmann::json::to_msgpack(response_object);
                return response;
            }
            // Key found, return success response with value
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["response"]["id"] = node.get_id();
            response_object["response"]["value"] = value;
        }
    }
    //-----------------------------------------------------
    if(method == "put") {
        // If ipc_mode is false, it means the "put" message is being processed from other nodes. In this case, the key-value pair is stored in the node's own key-value store using the node.store(key, value) function.
        if(ipc_mode == false) { // For Processing Put Requests from Other Nodes:
            assert(request_object["args"].is_object());
            auto params_object = request_object["args"];
            assert(params_object["key"].is_string());
            std::string key = params_object["key"];
            assert(params_object["value"].is_string());
            std::string value = params_object["value"];
        
            // Add the key-value pair to the key-value store
            code = (node.store(key, value) == false) 
                   ? static_cast<int>(KadResultCode::StoreFailed) 
                   : static_cast<int>(KadResultCode::Success);
            
            // Map keys to search terms for efficient search operations
            node.map(key, value);
        
            // Return success response // TODO: error reply
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["response"]["id"] = node.get_id();
            response_object["response"]["code"] = code;
            response_object["response"]["message"] = (code != 0) ? "Store failed" : "Success";
        } else { // For Sending Put Requests to Other Nodes
            // On the other hand, if ipc_mode is true, it means the "put" message is being sent from the local IPC client. In this case, the node.send_put(key, value) function is called to send the put message to the closest nodes in the routing table. Additionally, you can add a line of code to store the key-value pair in the local node's own hash table as well
            assert(request_object["args"].is_object());
            auto params_object = request_object["args"];
            assert(params_object["key"].is_string());
            std::string key = params_object["key"];
            assert(params_object["value"].is_string());
            std::string value = params_object["value"];

            // Send put messages to the closest nodes in your routing table (IPC mode)
            int put_messages_sent = node.send_put(key, value);
            code = (put_messages_sent <= 0) 
                   ? static_cast<int>(KadResultCode::StoreFailed) 
                   : static_cast<int>(KadResultCode::Success);
            std::cout << "Number of nodes you've sent a put message to: " << put_messages_sent << "\n";
                   
            // Store the key-value pair in your own node as well
            node.store(key, value);
            
            // Map keys to search terms for efficient search operations
            node.map(key, value);
        
            // Return success response
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["response"]["id"] = node.get_id();
            response_object["response"]["code"] = (code != 0) ? static_cast<int>(KadResultCode::StorePartial) : code;
            response_object["response"]["message"] = (code != 0) ? "Store failed" : "Success";
        }
    }
    //-----------------------------------------------------
    if(method == "map") {
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["key"].is_string());
        std::string key = params_object["key"];
        assert(params_object["value"].is_string());
        std::string value = params_object["value"];
        
        // Store indexing data in database on receiving a "map" request
        node.map(key, value);
    
        // Return success response
        response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
        response_object["response"]["id"] = node.get_id();
    }
    //-----------------------------------------------------
    if(method == "set" && ipc_mode) { // modify/update data
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["key"].is_string());
        std::string key = params_object["key"];
        assert(params_object["value"].is_string());
        std::string value = params_object["value"];
        
        // Retrieve the original data from the DHT
        std::string current_value = node.send_get(key);
        
        // Verify signature
        // ...
    }
    //-----------------------------------------------------
    if(method == "search" && ipc_mode) { // For value-based DHT lookups, but only works on data that your node has
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["what"].is_string());
        std::string what = params_object["what"].get<std::string>(); // what field=id, name, etc.
        assert(params_object["from"].is_string());
        std::string from = params_object["from"]; // from=user,product
        std::string condition; // condition=order by name ASC
        if(params_object.contains("condition")) {
            assert(params_object["condition"].is_string());
            condition = params_object["condition"];
        }
            
        std::vector<std::pair<std::string, std::string>> node_data = node.get_data();
        if(node_data.empty()) {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["response"]["id"] = node.get_id();
            response_object["response"]["result"] = nullptr;
        }
        
        if(!node_data.empty()) {
            std::vector<nlohmann::json> result_array;
            for (const auto& data : node_data) { 
                std::string value = data.second;
                nlohmann::json json;
                try {
                    json = nlohmann::json::parse(value);
                } catch (const nlohmann::json::parse_error& e) {
                    std::cerr << "JSON parsing error: " << e.what() << std::endl;
                    continue; // Skip to the next data if parsing fails
                }
                if (json.contains("metadata")) {
                    assert(json["metadata"].is_string());
                    std::string metadata = json["metadata"];
                    if (metadata == from) { // user, product, etc.
                        if(what == "*" || what == "all") {
                            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
                            response_object["response"]["id"] = node.get_id();
                            result_array.push_back(json); //json=entire object
                        } else {
                            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
                            response_object["response"]["id"] = node.get_id();
                            result_array.push_back(json[what]); // what=name, id, etc.
                        }
                        // Apply additional conditions or perform desired operations
                        // based on the matching metadata
                        // ...
                        // Example: Print the matched value
                        //std::cout << "Matched value: " << value << std::endl;
                    }
                }
            }
            response_object["response"]["result"] = result_array;
        }
    }
    //-----------------------------------------------------
    response_object["tid"] = tid; // transaction id - MUST be the same as the request object's id
    response = nlohmann::json::to_msgpack(response_object);
    return response;
}

//-----------------------------------------------------------------------------

std::string neroshop::msgpack::generate_secret(int length) {
    std::string secret(length, ' ');
    RAND_bytes((unsigned char*)&secret[0], length);
    return secret;
}

//-----------------------------------------------------------------------------

std::string neroshop::msgpack::generate_token(const std::string& node_id, const std::string& info_hash, const std::string& secret) {
    std::string token;
    uint8_t token_data[EVP_MAX_MD_SIZE];
    unsigned int token_length = 0;

    const EVP_MD* md = EVP_get_digestbyname("sha3-256");
    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();

    EVP_DigestInit_ex(md_ctx, md, nullptr);
    EVP_DigestUpdate(md_ctx, node_id.data(), node_id.size());
    EVP_DigestUpdate(md_ctx, info_hash.data(), info_hash.size());
    EVP_DigestUpdate(md_ctx, secret.data(), secret.size());
    EVP_DigestFinal_ex(md_ctx, token_data, &token_length);

    EVP_MD_CTX_free(md_ctx);

    token = std::string(reinterpret_cast<char*>(token_data), token_length);
    return token;
}


//-----------------------------------------------------------------------------
std::string neroshop::msgpack::generate_transaction_id() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::uint32_t> dis(0, std::numeric_limits<std::uint32_t>::max());
    std::uint32_t tid = dis(gen);
    std::array<std::uint8_t, 4> tid_bytes;
    tid_bytes[0] = static_cast<std::uint8_t>((tid >> 24) & 0xFF);
    tid_bytes[1] = static_cast<std::uint8_t>((tid >> 16) & 0xFF);
    tid_bytes[2] = static_cast<std::uint8_t>((tid >> 8) & 0xFF);
    tid_bytes[3] = static_cast<std::uint8_t>(tid & 0xFF);
    ////return std::string(reinterpret_cast<const char*>(tid_bytes.data()), 4);
    std::stringstream ss;
    for (const auto& b : tid_bytes) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    return ss.str().substr(0, 4); // take the first 4 characters only
}
//-----------------------------------------------------------------------------

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
