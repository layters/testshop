#include "msgpack.hpp"

#define JSON_USE_MSGPACK
#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include "../../version.hpp"
#include "../../tools/logger.hpp"
#include "../p2p/dht_rescode.hpp"
#include "../p2p/node.hpp"
#include "../p2p/routing_table.hpp"

namespace neroshop {

namespace msgpack {

std::vector<uint8_t> process(const std::vector<uint8_t>& request, Node& node, bool ipc_mode) {
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
        response_object["error"]["code"] = static_cast<int>(DhtResultCode::ParseError); // "code" MUST be an integer
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
        assert(params_object["target"].is_string()); // target (node id or key) sought after by the querying node
        std::string target = params_object["target"].get<std::string>();
        
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
    if(method == "get_providers") {
        std::cout << "message type is a get_providers\n"; // 
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["data_hash"].is_string()); // info hash
        std::string data_hash = params_object["data_hash"].get<std::string>();
        
        response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
        response_object["response"]["id"] = node.get_id();
        // Check if the queried node has peers for the requested infohash
        std::vector<Peer> peers = node.get_providers(data_hash);
        if(peers.empty()) {
            // WARNING!!! THIS CODE BLOCKS THE GUI.
            // If the queried node has no peers for the requested infohash,
            // return the K closest nodes in the routing table to the requested infohash
            std::vector<Node*> closest_nodes = node.find_node(data_hash, NEROSHOP_DHT_MAX_CLOSEST_NODES);
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
            response_object["response"]["peers"] = peers_array; // If the queried node has peers for the infohash, they are returned in a key "values" as a list of strings. Each string containing "compact" format peer information for a single peer
        }
    }
    //-----------------------------------------------------
    if(method == "get") { // For Sending Get Requests to Other Nodes and For Processing Get Requests from Other Nodes
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["key"].is_string());
        std::string key = params_object["key"].get<std::string>();
        
        // To get network status
        if(key == "status") {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["response"]["id"] = node.get_id();
            response_object["response"]["connected_peers"] = node.get_peer_count();
            response_object["response"]["active_peers"] = node.get_active_peer_count();
            response_object["response"]["idle_peers"] = node.get_idle_peer_count();
            response_object["tid"] = tid;
            response = nlohmann::json::to_msgpack(response_object);
            return response;
        }
                        
        // Send get messages to the closest nodes in your routing table (IPC mode)
        // But first, look up the value in the node's own hash table
        std::string value = node.send_get(key);
            
        // Key not found, return error response
        if (value.empty()) {
            code = static_cast<int>(DhtResultCode::RetrieveFailed);
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["error"]["id"] = node.get_id();
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
    //-----------------------------------------------------
    if(method == "put" && ipc_mode == false) { // For Processing Put Requests from Other Nodes - If ipc_mode is false, it means the "put" message is being processed from other nodes. In this case, the key-value pair is stored in the node's own key-value store using the node.store(key, value) function.
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["key"].is_string());
        std::string key = params_object["key"].get<std::string>();
        assert(params_object["value"].is_string());
        std::string value = params_object["value"].get<std::string>();
        
        // Add the key-value pair to the key-value store
        code = (node.store(key, value) == false) 
               ? static_cast<int>(DhtResultCode::StoreFailed) 
               : static_cast<int>(DhtResultCode::Success);
            
        if(code == 0) {
            // Map keys to search terms for efficient search operations
            node.map(key, value);
        }
        
        // Return response or error
        if(code != static_cast<int>(DhtResultCode::Success)) {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["error"]["id"] = node.get_id();
            response_object["error"]["code"] = code;
            response_object["error"]["message"] = get_dht_result_code_as_string(static_cast<DhtResultCode>(code));
        } else {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["response"]["id"] = node.get_id();
        }
    }
    //-----------------------------------------------------
    if(method == "map") {
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["key"].is_string());
        std::string key = params_object["key"].get<std::string>();
        assert(params_object["value"].is_string());
        std::string value = params_object["value"].get<std::string>();
        
        // Since we are not storing in hash table, we must validate the data before mapping it
        if(node.validate(key, value)) {
        
            // Store indexing data in database on receiving a "map" request
            node.map(key, value);
        }
    
        // Return success response
        response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
        response_object["response"]["id"] = node.get_id();
    }
    //-----------------------------------------------------
    if((method == "set" || method == "put") && ipc_mode == true) { // For Sending Put Requests to Other Nodes - If ipc_mode is true, it means the "put" message is being sent from the local IPC client. In this case, the node.send_put(key, value) function is called to send the put message to the closest nodes in the routing table. Additionally, you can add a line of code to store the key-value pair in the local node's own hash table as well
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["key"].is_string());
        std::string key = params_object["key"].get<std::string>();
        assert(params_object["value"].is_string());
        std::string value = params_object["value"].get<std::string>();
        
        // Send put messages to the closest nodes in your routing table (IPC mode)
        int put_messages_sent = node.send_put(key, value);
        code = (put_messages_sent <= 0) 
               ? static_cast<int>(DhtResultCode::StoreFailed) 
               : static_cast<int>(DhtResultCode::Success);
        std::cout << "Number of nodes you've sent a put message to: " << put_messages_sent << "\n";
        
        if((put_messages_sent < NEROSHOP_DHT_REPLICATION_FACTOR) && (put_messages_sent > 0)) {
            code = static_cast<int>(DhtResultCode::StorePartial);
        }
                   
        // Store the key-value pair in your own node as well
        if(node.store(key, value)) {
            if(put_messages_sent == 0) { 
                code = static_cast<int>(DhtResultCode::StoreToSelf);
            }
            
            // Store your local client's own data on-disk (in case of outage)
            node.cache(key, value);
            
            // Map keys to search terms for efficient search operations
            node.map(key, value);
        }
        
        // Return response or error
        if((code == static_cast<int>(DhtResultCode::Success)) || (code == static_cast<int>(DhtResultCode::StorePartial))) {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["response"]["id"] = node.get_id();
        } else {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["error"]["id"] = node.get_id();
            response_object["error"]["code"] = code;
            response_object["error"]["message"] = get_dht_result_code_as_string(static_cast<DhtResultCode>(code));
        }
    }
    //-----------------------------------------------------
    response_object["tid"] = tid; // transaction id - MUST be the same as the request object's tid
    response = nlohmann::json::to_msgpack(response_object);
    return response;
}

//-----------------------------------------------------------------------------

std::string generate_secret(int length) {
    std::string secret(length, ' ');
    RAND_bytes((unsigned char*)&secret[0], length);
    return secret;
}

//-----------------------------------------------------------------------------

std::string generate_token(const std::string& node_id, const std::string& data_hash, const std::string& secret) {
    std::string token;
    uint8_t token_data[EVP_MAX_MD_SIZE];
    unsigned int token_length = 0;

    const EVP_MD* md = EVP_get_digestbyname("sha3-256");
    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();

    EVP_DigestInit_ex(md_ctx, md, nullptr);
    EVP_DigestUpdate(md_ctx, node_id.data(), node_id.size());
    EVP_DigestUpdate(md_ctx, data_hash.data(), data_hash.size());
    EVP_DigestUpdate(md_ctx, secret.data(), secret.size());
    EVP_DigestFinal_ex(md_ctx, token_data, &token_length);

    EVP_MD_CTX_free(md_ctx);

    token = std::string(reinterpret_cast<char*>(token_data), token_length);
    return token;
}


//-----------------------------------------------------------------------------
std::string generate_transaction_id() {
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

bool send_data(int sockfd, const std::vector<uint8_t>& packed) {
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

std::string receive_data(int sockfd) {
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

}

}
