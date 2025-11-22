#include "json_rpc.hpp"

#include "../../database/database.hpp"
#include "../../tools/logger.hpp"

#if defined(NEROSHOP_USE_QT)
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#else
#include <nlohmann/json.hpp>
#endif

#include <random>
#include <algorithm>
// uncomment to disable assert()
// #define NDEBUG
#include <cassert>

namespace neroshop {

namespace rpc {

std::string json_translate(const std::string& sql) {
    std::string request = "";
    std::string random_id = json_generate_random_id();
    #if defined(NEROSHOP_USE_QT)
    QJsonObject request_object; // JSON-RPC Request object
    request_object.insert(QString("jsonrpc"), QJsonValue("2.0"));
    request_object.insert(QString("method"), QJsonValue("query"));////QJsonValue("query." + QString::fromStdString(method_type)));
    QJsonObject params_object;////QJsonArray params_array; // can be an array or object, but object is preferred as it can hold both key and value
    params_object.insert(QString("sql"), QJsonValue(QString::fromStdString(sql)));
    request_object.insert(QString("params"), QJsonValue(params_object)); // "params" MAY be omitted
    request_object.insert(QString("id"), QJsonValue(QString::fromStdString(random_id))); // https://stackoverflow.com/questions/2210791/json-rpc-what-is-the-id-for
    // Convert JSON to string
    QJsonDocument json_doc(request_object);
    QString json_str = json_doc.toJson();////json_doc.toJson(QJsonDocument::Compact);
    request = json_str.toStdString();
    #else
    nlohmann::json json;
    json["jsonrpc"] = "2.0";
    json["method"] = "query";
    json["params"]["sql"] = sql;
    json["id"] = random_id;
    // Dump JSON to string
    request = json.dump(4);////json.dump();
    #endif
    #ifdef NEROSHOP_DEBUG0
    std::cout << "\"" << sql << "\" has been translated to: \n" << request << std::endl;
    #endif    
    return request;
}
//----------------------------------------------------------------
std::string json_translate(const std::string& sql, const std::vector<std::string>& args) {
    std::string request = "";
    assert(json_get_query_method(sql) != "SELECT" && "SELECT statements with arguments are not supported. Please use the non-query functions (e.g. get_*() functions)"); // This is due to a lack of a callback in execute_params which uses sqlite3_prepare, sqlite3_step functions. On the other hand, execute uses sqlite_exec which does have a callback that returns the result from a SELECT statement
    std::string random_id = json_generate_random_id();
    #if defined(NEROSHOP_USE_QT)
    QJsonObject request_object; // JSON-RPC Request object
    request_object.insert(QString("jsonrpc"), QJsonValue("2.0"));
    request_object.insert(QString("method"), QJsonValue("query"));
    QJsonObject params_object;
    params_object.insert(QString("sql"), QJsonValue(QString::fromStdString(sql))); // eg. "SELECT * FROM products WHERE condition = $1;"
    params_object.insert(QString("count"), static_cast<int>(args.size()));
    for(int index = 0; index < args.size(); index++) {
        std::string arg_key = std::to_string(index + 1);
        std::string arg_value = args[index];
        params_object.insert(QString::fromStdString(arg_key), QJsonValue(QString::fromStdString(arg_value)));
        std::cout << "" << (index + 1) << "= " << arg_value << "\n";
    }
    request_object.insert(QString("params"), QJsonValue(params_object));
    request_object.insert(QString("id"), QJsonValue(QString::fromStdString(random_id)));
    // Convert JSON to string
    QJsonDocument json_doc(request_object);
    QString json_str = json_doc.toJson();////json_doc.toJson(QJsonDocument::Compact);
    request = json_str.toStdString();
    #else
    nlohmann::json json;
    json["jsonrpc"] = "2.0";
    json["method"] = "query";
    json["params"]["sql"] = sql;
    json["params"]["count"] = args.size();
    for(int index = 0; index < args.size(); index++) {
        std::string arg_key = std::to_string(index + 1);
        std::string arg_value = args[index];
        json["params"][arg_key] = arg_value;
    }
    json["id"] = random_id;
    // Dump JSON to string
    request = json.dump(4);////json.dump();    
    #endif
    #ifdef NEROSHOP_DEBUG0
    std::cout << "\"" << sql << "\" has been translated to: \n" << request << std::endl;
    #endif        
    return request;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
std::string json_process(const std::string& request) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    std::string response = "";
    // Okay, so we (the server) have received a request from the client
    #if defined(NEROSHOP_USE_QT)
    // Now we can process (parse) the request
    QJsonObject response_object; // JSON-RPC Response object
    QJsonParseError parse_error;
    const auto json_doc = QJsonDocument::fromJson(QString::fromStdString(request).toUtf8(), &parse_error);
    if (parse_error.error != QJsonParseError::NoError) {
        neroshop::log_error("Error parsing client request");
        response_object.insert(QString("jsonrpc"), QJsonValue("2.0"));
        // "result" MUST NOT exist if there was an error invoking the method
        QJsonObject error_object; // "error" MUST be an Object as defined in section 5.1
        error_object.insert(QString("code"), QJsonValue(-32700)); // "code" MUST be an integer
        error_object.insert(QString("message"), QJsonValue("Parse error"));
        QString data;
        switch(parse_error.error) {
            case QJsonParseError::UnterminatedObject:
                data = "Unterminated object";
                break;
            case QJsonParseError::MissingNameSeparator:
                data = "Missing name separator";
                break;
            case QJsonParseError::UnterminatedArray:
                data = "Unterminated array";
                break;
            case QJsonParseError::MissingValueSeparator:
                data = "Missing value separator";
                break;
            case QJsonParseError::IllegalValue:
                data = "Illegal value";
                break;
            case QJsonParseError::TerminationByNumber:
                data = "Termination by number";
                break;
            case QJsonParseError::IllegalNumber:
                data = "Illegal number";
                break;
            case QJsonParseError::IllegalEscapeSequence:
                data = "Illegal escape sequence";
                break;
            case QJsonParseError::IllegalUTF8String:
                data = "Illegal utf8 string";
                break;
            case QJsonParseError::UnterminatedString:
                data = "Unterminated string";
                break;
            case QJsonParseError::MissingObject:
                data = "Missing object";
                break;
            case QJsonParseError::DeepNesting:
                data = "Deep nesting";
                break;
            case QJsonParseError::DocumentTooLarge:
                data = "Document too large";
                break;
            case QJsonParseError::GarbageAtEnd:
                data = "Garbage at end";
                break;                                                                                                                                                                                                                
        }
        if(!data.isEmpty()) error_object.insert(QString("data"), QJsonValue("Reason: " + data)); // additional information about the error which may be omitted
        response_object.insert(QString("error"), QJsonValue(error_object));
        response_object.insert(QString("id"), QJsonValue(QJsonValue::Null)); // "id" MUST be Null if there was an error in detecting the id in the Request object (e.g. Parse error/Invalid Request)
        // Return a response with an error_object
        response = QJsonDocument(response_object).toJson().toStdString();
        #ifdef NEROSHOP_DEBUG0
        std::cout << "Response output:\n\033[91m" << response << "\033[0m\n";
        #endif
        return response;
    }
    //  Retrieve keys and values from request_object
    #ifdef NEROSHOP_DEBUG
    std::cout << /*"Request received:\n" << */"\033[33m" << json_doc.toJson(QJsonDocument::Compact).toStdString() << "\033[0m\n";
    #endif
    QJsonObject request_object = json_doc.object();
    QJsonValue jsonrpc_version = request_object.value("jsonrpc");
    assert(jsonrpc_version.isString()); 
    assert(jsonrpc_version.toString() == "2.0");
    QJsonValue method = request_object.value("method");
    assert(method.isString());
    QJsonValue params = request_object.value("params"); // "params" MAY be omitted
    QJsonValue id = request_object.value("id");
    if(id.isUndefined()) {
        // a request object without an "id" member is assumed to be NOTIFICATION ...
        // a Notification signifies the Client's lack of interest in the corresponding Response object, and as such no Response object needs to be returned to the client. The Server MUST NOT reply to a Notification, including those that are within a batch request.
        // exit function, with an empty string object perhaps
        //return "";
    }
    // Execute the request (run function and get return value)
    int code = 0;
    if(method.toString() == "query") { 
        assert(params.isObject());
        QJsonObject param_object = params.toObject();
        QJsonValue sql = param_object.value("sql");
        assert(sql.isString());
        QJsonValue arg_count = param_object.value("count");
        bool has_args = !arg_count.isUndefined();
        if(has_args) { 
            assert((json_get_query_method(sql.toString().toStdString()) != "SELECT") && "SELECT statements with arguments are not allowed. Please use a non-query method (i.e. get_*() functions)");
            std::vector<std::string> args;
            for(int index = 0; index < arg_count.toInt(); index++) {
                QJsonValue argument = param_object.value(QString::fromStdString(std::to_string(index + 1)));
                assert(argument.isString());
                args.push_back(argument.toString().toStdString()); // Store arguments in std::vector
            }
            assert(args.size() == arg_count.toInt()); // Make sure the number of args is the same as the count
            code = database->execute_params(sql.toString().toStdString(), args); // Execute query with n args
        }
        else if(!has_args) { // No args
            code = database->execute(sql.toString().toStdString()); // Execute query with zero args
        }
    }
    //-------------------------------------------------------
    // TODO: deal with normal non-query functions later
    else { std::cout << "method= " << method.toString().toStdString() << "\n";
        // TODO: add support for arrays (e.g "params": ["hello", 1])
        ////if(params.isArray()) {//if(params.isObject()) {//if(params.isUndefined()) {}
    }
    //-------------------------------------------------------
    // After execution, get the result or error, if any and then return it as a JSON-RPC response message
    response_object.insert(QString("jsonrpc"), jsonrpc_version);
    // result
    if(code == 0) {
        if(method.toString() == "query") {
            std::string query = params.toObject().value("sql").toString().toStdString();
            std::string query_method = json_get_query_method(query);
            if(query_method == "SELECT") { // if method is a SELECT query
                std::string result = database->get_select();
                // Create a JSON document from the result string and check for any errors
                QJsonParseError parse_row_error;
                QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(result).toUtf8(), &parse_row_error);
                if (parse_row_error.error != QJsonParseError::NoError) {
                    neroshop::log_error("Error parsing SELECT rows"); return "";
                }
                // A JSON Document can either be an array or an object
                assert(/*doc.isObject() || */doc.isArray()); // Should be an array of rows (objects) as objects cannot be looped through but arrays can
                QJsonValue result_value = (doc.isObject()) ? QJsonValue(doc.object()) : QJsonValue(doc.array());
                response_object.insert(QString("result"), result_value);
            }
            else {
                response_object.insert(QString("result"), QJsonValue(code)); // All good, zero errors found!
            }
        }
    }
    // error
    if(code != 0) {
        QJsonObject error_object; // "error" MUST be an Object as defined in section 5.1
        error_object.insert(QString("code"), QJsonValue(code)); // if method is query then this would be an SQLite result code. See https://www.sqlite.org/rescode.html#ok
        QString message;
        if(method.toString() == "query") {
            switch(code) {
                case SQLITE_ERROR: // 1
                    message = "SQLITE_ERROR";
                    break;
                // TODO: add the other SQLite result error codes
                default: 
                    message = "SQLITE_ERROR"; // 1
                    break;
            }
            error_object.insert(QString("message"), QJsonValue(message));
            std::string data = database->get_error().second;
            error_object.insert(QString("data"), QJsonValue(QString::fromStdString(data))); // optional
        }
        response_object.insert(QString("error"), QJsonValue(error_object)); // "error" MUST NOT exist if there was no error triggered during invocation.
    }
    response_object.insert(QString("id"), id); // "id" MUST be the same as the request object's id
    // Print response (for debugging purposes)
    response = QJsonDocument(response_object).toJson(/*QJsonDocument::Compact*/).toStdString();
    #ifdef NEROSHOP_DEBUG0
    std::cout << "Response output:\n" << ((code != 0) ? "\033[91m" : "\033[32m") << response << "\033[0m\n";    
    #endif
    #else
    // Process (parse) the request
    nlohmann::json response_object;
    nlohmann::json request_object;
    try {
        request_object = nlohmann::json::parse(request);
    }
    catch(nlohmann::json::parse_error& exception) {
        neroshop::log_error("Error parsing client request");
        response_object["jsonrpc"] = "2.0";
        response_object["error"]["code"] = -32700; // "code" MUST be an integer
        response_object["error"]["message"] = "Parse error";
        response_object["error"]["data"] = exception.what(); // A Primitive (non-object) or Structured (array) value which may be omitted
        response_object["id"] = nullptr;
        response = response_object.dump(4);
        #ifdef NEROSHOP_DEBUG0
        std::cout << "Response output:\n\033[91m" << response << "\033[0m\n";
        #endif
        return response;
    }
    //  Retrieve keys and values from request_object
    #ifdef NEROSHOP_DEBUG
    std::cout << /*"Request received:\n" << */"\033[33m" << request_object.dump() << "\033[0m\n";
    #endif
    assert(request_object.is_object());
    assert(request_object["jsonrpc"].is_string());
    std::string jsonrpc_version = request_object["jsonrpc"];
    assert(jsonrpc_version == "2.0");
    assert(request_object["method"].is_string());
    std::string method = request_object["method"];
    // "params" MAY be omitted
    if(!request_object.contains("id")) {
        std::cout << "No id found, hence a notification that will not receive a response from the server\n";return "";
    }
    auto id = request_object["id"]; // could either a string, integer or null
    int code = 0;
    if(method == "query") {
        assert(request_object["params"].is_object());
        auto param_object = request_object["params"];
        assert(param_object["sql"].is_string());
        std::string sql = param_object["sql"];
        bool has_args = param_object.contains("count");
        if(has_args) { 
            assert((json_get_query_method(sql) != "SELECT") && "SELECT statements with arguments are not allowed. Please use a non-query method (i.e. get_*() functions)");
            std::vector<std::string> args;
            for(int index = 0; index < param_object.count("count"); index++) {
                auto argument = param_object[std::to_string(index + 1)];
                assert(argument.is_string());
                std::cout << argument << " (arg: " << (index + 1) << ")" << std::endl;
                args.push_back(argument); // Store arguments in std::vector
            }
            assert(args.size() == param_object.count("count")); // Make sure the number of args is the same as the count
            code = database->execute_params(sql, args); // Execute query with n args
        }
        else if(!has_args) { // No args
            code = database->execute(sql); // Execute query with zero args
        }
    }
    //-------------------------------------------------------
    //if(method == "") {
    //}
    //-------------------------------------------------------
    // After execution, get the result or error, if any and then return it as a JSON-RPC response message
    response_object["jsonrpc"] = jsonrpc_version;
    // result
    if(code == 0) {
        if(method == "query") {
            auto param_object = request_object["params"];
            std::string query = param_object["sql"];
            std::string query_method = json_get_query_method(query);
            if(query_method == "SELECT") { // if method is a SELECT query
                std::string result = database->get_select();
                // Create a JSON document from the result string and check for any errors
                nlohmann::json rows = nlohmann::json::parse(result);
                if(!nlohmann::json::accept(result)) {
                    neroshop::log_error("Error parsing SELECT rows"); return "";
                }
                auto result_value = rows; // Should be an array of rows (objects) as objects cannot be looped through but arrays can
                response_object["result"] = result_value;
            }
            else {
                response_object["result"] = code; // All good, zero errors found!
            }
        }
    }
    // error
    if(code != 0) {
        nlohmann::json error_object; // "error" MUST be an Object as defined in section 5.1
        error_object["code"] = code; // if method is query then this would be an SQLite result code. See https://www.sqlite.org/rescode.html#ok
        std::string message;
        if(method == "query") {
            switch(code) {
                case SQLITE_ERROR: // 1
                    message = "SQLITE_ERROR";
                    break;
                // TODO: add the other SQLite result error codes
                default: 
                    message = "SQLITE_ERROR"; // 1
                    break;
            }
            error_object["message"] = message;
            std::string data = database->get_error().second;
            error_object["data"] = data; // "data" may be ommited
        }
        response_object["error"] = error_object; // "error" MUST NOT exist if there was no error triggered during invocation.
    }
    response_object["id"] = id; // "id" MUST be the same as the request object's id
    // Print response (for debugging purposes)
    response = response_object.dump(4);
    #ifdef NEROSHOP_DEBUG0
    std::cout << "Response output:\n" << ((code != 0) ? "\033[91m" : "\033[32m") << response << "\033[0m\n";    
    #endif
    #endif
    return response;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
void json_request(const std::string& json) { // TODO: this function should return a json_rpc response (string) from the server
    // Get the json which is basically a translated sqlite query or a translated c++ function name (string) + args
    // Send the request_object to the server
    ////zmq_send (requester, request.c_str(), request.size(), 0);
    // Lastly, the server will then execute the data and return any results
} // Usage: json_request(json_translate("SELECT * FROM users;"));
//----------------------------------------------------------------
void json_request_batch(const std::vector<std::string>& json_batch) {
}
//----------------------------------------------------------------
//----------------------------------------------------------------
void json_respond(const std::string& json) {
    std::string response = json_process(json);
    if(response.empty()) return;
    // Reply to client with the response object
    ////zmq_send (responder, response.c_str(), response.size(), 0);    
}
//----------------------------------------------------------------
void json_respond_batch(const std::vector<std::string>& json_batch) {
}
//----------------------------------------------------------------
//----------------------------------------------------------------
std::string json_get_query_method(const std::string& sql) {
	std::string first_word = sql.substr(0, sql.find_first_of(" "));
	std::transform(first_word.begin(), first_word.end(), first_word.begin(), [](unsigned char c){ return std::toupper(c); }); // query_methods are stored in UPPER case strings so the same must be applied to the first_word
    if(json_is_query_method(first_word)) return first_word;
    return "";
}
//----------------------------------------------------------------
bool json_is_query_method(const std::string& query_method) {
    return (std::find(query_methods.begin(), query_methods.end(), query_method) != query_methods.end());
}
//----------------------------------------------------------------
bool json_is_method(const std::string& method) {
    return (methods.count(method) > 0);
}
//----------------------------------------------------------------
bool is_json_rpc(const std::string& str) {
#if defined(NEROSHOP_USE_QT)
    // Parse the string as a JSON document
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(str).toUtf8(), &error);

    // Check if parsing was successful and the root element is an object
    if (error.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject obj = doc.object();

        // Check if the object has a "jsonrpc" field with value "2.0"
        if (obj.contains("jsonrpc") && obj["jsonrpc"].isString() && obj["jsonrpc"].toString() == "2.0"
            && obj.contains("method") && obj["method"].isString()) {
            return true;
        }
    }
#else
    try {
        nlohmann::json j = nlohmann::json::parse(str);
        if (j.is_object() && j.contains("jsonrpc") && j.contains("method")
            && j["jsonrpc"].is_string() && j["method"].is_string()
            && j["jsonrpc"].get<std::string>() == "2.0") {
            return true;
        }
    }
    catch (const std::exception& e) {
        return false;
    }
#endif    
    return false;
}
//----------------------------------------------------------------
std::string json_generate_random_id() {
    // Generate random number for id (id can be either a string or an integer or null which is not recommended)
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(1, 9); // define the range
    std::string random_id;
    for(int n = 0; n < 10; ++n) {
        int random_integer = distr(gen); // generate numbers
        random_id = random_id + std::to_string(random_integer); // append 10 different numbers to generate a unique id
    }    
    return random_id;
}

} // namespace rpc

} // namespace neroshop
