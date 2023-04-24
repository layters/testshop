#include "krpc.hpp"

#include <random>
#include <iomanip> // std::set*

// refer to KRPC Protocol specs: http://www.bittorrent.org/beps/bep_0005.html

std::string neroshop::rpc::krpc::process(const std::string& request, const std::string& responding_node_id) {
    std::string response = "";
    size_t pos = 0;
    // Parse bencoded string
    try {
        bencode::parse_bencoded(request, pos);
    } catch (std::exception& e) {
        // Return an error message if decoding the request fails
        bencode_dict error_dict;
        error_dict["t"] = generate_transaction_id(); // every message is required to have a tid regardless
        error_dict["y"] = "e"; // "e" = error
        error_dict["e"] = bencode_list { 201, std::string("parse_bencoded error: ") + e.what() }; // The value of "e" is a list. The first element is an integer representing the error code. The second element is a string containing the error message. Errors are sent when a query cannot be fulfilled.
        response = bencode::encode(error_dict);
        #ifdef NEROSHOP_DEBUG0
        std::cout << "Response output:\n" << "\033[91m" << response << "\033[0m\n";    
        #endif
        return response;
    }
    pos = 0; // reset position
    bencode_dict request_dict = bencode::decode_dict(request, pos);    
    //  Retrieve keys and values from request_object
    #ifdef NEROSHOP_DEBUG
    std::cout << /*"Request received:\n" << */"\033[33m" << request << "\033[0m\n";
    #endif    
    bencode_dict response_dict;
    // Get message type
    if (request_dict.count("y") > 0) {
        // q = query (if y = q then it contains two additional keys; "q" and "a". Key "q" has a string value containing the method name of the query. Key "a" has a dictionary value containing named arguments to the query.)
        // r = response (if y = r then it contains one additional key "r". The value of "r" is a dictionary containing named return values. Response messages are sent upon successful completion of a query.)
        // e = error
        // v = version (should be included in every message with a client version string; not needed)
    }
    // Get the query type, if message type ("y") is a query ("q")
    int code = 0;
    if (request_dict.count("q") > 0) {
        std::string query_type = std::get<std::string>(request_dict["q"]);
        if(query_type == "ping") { 
            response_dict["t"] = std::get<std::string>(request_dict["t"]);
            response_dict["y"] = "r"; // "r" = response
            response_dict["r"] = "pong";
            response_dict["r.id"] = responding_node_id; // "r" must be a dict with a single key "id" containing the responding node's id
            
        }
        if(query_type == "find_node") {
            response_dict["t"] = std::get<std::string>(request_dict["t"]);
            response_dict["y"] = "r";
            //response_dict["r.id"] = ""; // "r" is a dict with two keys, "nodes" containing a string of nodes' info (address:ip) or the closests good nodes in the routing table, and "id" containing the ID of the nodes sought by the queryer.
            //response_dict["r.nodes"] = "";
        }
        if(query_type == "get_peers") {
        }
        if(query_type == "announce_peer") {
        }
        if(query_type == "get") {
        }
        if(query_type == "put") {
        }
        /*if(query_type == "") {
        }*/
    }
    // Get the transaction id
    if (request_dict.count("t") > 0) {
    }
    //-----------------------------------------------------------------------
    
    //-----------------------------------------------------------------------
    // Construct a response
    //response_dict["t"] = <transaction id we got from the request>;
    //response_dict["y"] = "r"; // "r" is for a response message_type ("y")
    //response_dict["q"] = // can be "pong", "nodes", etc.
    response = bencode::encode(response_dict);
    #ifdef NEROSHOP_DEBUG0
    std::cout << "Response output:\n" << ((code != 0) ? "\033[91m" : "\033[32m") << response << "\033[0m\n";    
    #endif
    return response;
}

bool neroshop::rpc::is_bencoded(const std::string& str) {
    if(bencode::is_bencoded(str)) {
        return true;
    }
    return false;
}

std::string neroshop::rpc::krpc::generate_transaction_id() {
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
