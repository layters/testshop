#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <any> // std::any
#include <functional> // std::function
// TODO: translate method names like get_product_ratings (along with the args) into json request strings via command line
namespace neroshop {

namespace rpc {
    bool is_json_rpc(const std::string& str);
    // TODO: map names (strings) to functions
    static const std::unordered_map<std::string, std::function<void(const std::string&)>> methods = {//static std::unordered_map<std::string, std::function<void(int)>> methods_cmd
        { "query", nullptr },
        { "eat", [](const std::string& what){ std::cout << "I am eating " << what << "\n"; } } // For testing only
    };
    static const std::vector<std::string> query_methods = { 
        "SELECT", // extracts data from a database
        "UPDATE", // updates data in a database
        "DELETE", // deletes data from a database
        "INSERT", // "INSERT INTO" - inserts new data into a database
        "CREATE", // "CREATE DATABASE" - creates a new database // "CREATE TABLE" - creates a new table // "CREATE INDEX" - creates an index (search key)
        "ALTER",  // "ALTER DATABASE" - modifies a database // "ALTER TABLE" - modifies a table
        "DROP",   // "DROP TABLE" - deletes a table // "DROP INDEX" - deletes an index
    };
    extern std::string json_generate_random_id();
    extern std::string json_get_query_method(const std::string& sql);
    extern bool json_is_query_method(const std::string& query_method);
    extern bool json_is_method(const std::string& method);
    extern std::string json_translate(const std::string& sql); // converts an sqlite query to a json_rpc request message
    extern std::string json_translate(const std::string& sql, const std::vector<std::string>& args);
    template <typename... Args>
    static decltype (auto) translate_cmd(const std::string& method_name, Args&&... args) { // converts a method (string) and its argument(s) into a json_rpc response message via the command line
        // accessing the number of args: static const size_t arg_count = sizeof...(Args);
        //if(method == "get_whatever") { ...
        // accessing the arguments: functions[name](std::forward<Args>(args)...);
        ////methods["eat"](std::forward<Args>(args)...);
        return nullptr; // TEMPORARY
    }
    extern std::string json_process(const std::string& request); // (server) processes a request from the client
    extern void json_request(const std::string& json); // for client - to send requests
    extern void json_request_batch(const std::vector<std::string>& json_batch); // sends a batch of json_rpc requests to the server and expects a batch of json_rpc responses
    extern void json_respond(const std::string& json); // for server - to respond to requests
    extern void json_respond_batch(const std::vector<std::string>& json_batch);
} // namespace rpc

}
