#pragma once

#if defined(NEROSHOP_USE_QT)
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#else
#include <nlohmann/json.hpp>
#endif

#include <iostream>
#include <random>
// uncomment to disable assert()
// #define NDEBUG
////#include <cassert>
// neroshop JSON-RPC API // TODO: translate method names like get_product_ratings (along with the args) into json request strings via command line
namespace neroshop {

namespace rpc {
    static std::vector<std::string> methods = {};//static std::unordered_map<std::string, std::function<void(int)>> methods_cmd
    static std::vector<std::string> sql_methods = { // TODO: rename to query_type?
        "SELECT", // extracts data from a database
        "UPDATE", // updates data in a database
        "DELETE", // deletes data from a database
        "INSERT", // "INSERT INTO" - inserts new data into a database
        "CREATE", // "CREATE DATABASE" - creates a new database // "CREATE TABLE" - creates a new table // "CREATE INDEX" - creates an index (search key)
        "ALTER",  // "ALTER DATABASE" - modifies a database // "ALTER TABLE" - modifies a table
        "DROP",   // "DROP TABLE" - deletes a table // "DROP INDEX" - deletes an index
    };
    static std::string get_method(const std::string& sql);
    static bool is_valid_method(const std::string& method);
    static std::string translate(const std::string& sql); // converts an sqlite query to a json_rpc request message
    static std::string translate(const std::string& sql, const std::vector<std::string>& args);
    template <typename... Args>
    static std::string translate_cmd(const std::string& method, Args&&... args) { // converts a method (string) and its argument(s) into a json_rpc response message via the command line
        // accessing the number of args: static const size_t arg_count = sizeof...(Args);
        //if(method == "get_whatever") { ...
        // accessing the arguments: functions[name](std::forward<Args>(args)...);
        return ""; // TEMPORARY
    }
    static void request(const std::string& json); // for client - to send requests
    static void request_batch(const std::vector<std::string>& json_batch); // sends a batch of json_rpc requests to the server and expects a batch of json_rpc responses
    static void respond(const std::string& json); // for server - to respond to requests
    static void respond_batch(const std::vector<std::string>& json_batch);
}

}
