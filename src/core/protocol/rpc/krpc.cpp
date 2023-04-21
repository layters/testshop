#include "krpc.hpp"

std::string neroshop::rpc::krpc_process(const std::string& request) {
    std::string response = "";
    size_t pos = 0;
    bencode_dict request_dict = bencode::decode_dict(request, pos);
    // TODO: parse bencode first before doing anything and if there is an error parsing, return an error message type ("y" = "e")
    //  Retrieve keys and values from request_object
    #ifdef NEROSHOP_DEBUG0
    std::cout << "Request received:\n\033[33m" << request << "\033[0m\n";
    #endif    
    bencode_dict response_dict;
    // Get message type
    if (request_dict.count("y") > 0) {
        // q = query
        // r = response
        // e = error
    }
    // Get the query type, if message type ("y") is a query ("q")
    int code = 0;
    if (request_dict.count("q") > 0) {
        std::string query_type = std::get<std::string>(request_dict["q"]);
        if(query_type == "ping") { 
            response_dict["t"] = std::get<std::string>(request_dict["t"]);
            response_dict["y"] = "r"; // "r" is for a response message_type ("y")
            response_dict["q"] = "pong";
        }
        if(query_type == "find_node") {
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


