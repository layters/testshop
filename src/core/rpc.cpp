#include "rpc.hpp"

std::string neroshop::rpc::translate(const std::string& sql) {
    std::string request = "";
    #if defined(NEROSHOP_USE_QT)
    QJsonObject request_object; // JSON-RPC Request object
    
    request_object.insert(QString("jsonrpc"), QJsonValue("2.0"));
    request_object.insert(QString("method"), QJsonValue("query"));////QJsonValue(QString::fromStdString(method_type)));
    QJsonObject params_object;////QJsonArray params_array; // can be an array or object
    params_object.insert(QString("sql"), QJsonValue(QString::fromStdString(sql)));////params_array.insert(params_array.size(), QJsonValue("arg1")); // based on the number of parameter args
    request_object.insert(QString("params"), QJsonValue(params_object));
    // Generate random number for id (id can be either a string or an integer or null which is not recommended)
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(1, 9); // define the range
    std::string random_id;
    for(int n = 0; n < 10; ++n) {
        int random_integer = distr(gen); // generate numbers
        random_id = random_id + std::to_string(random_integer); // append 10 different numbers to generate a unique id
    }
    request_object.insert(QString("id"), QJsonValue(QString::fromStdString(random_id))); // https://stackoverflow.com/questions/2210791/json-rpc-what-is-the-id-for
    // Convert JSON to string then display it (for debugging purposes)
    QJsonDocument json_doc(request_object);
    QString json_str = json_doc.toJson();////(QJsonDocument::Compact); // https://doc.qt.io/qt-6/qjsondocument.html#JsonFormat-enum
    request = json_str.toStdString();
    #else
    nlohmann::json json;
    json["jsonrpc"] = "2.0";
    json["method"] = "query";////get_method(sql);
    json["params"]["sql"] = sql;
    // Generate random number for id (id can be either a string or an integer or null which is not recommended)
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(1, 9); // define the range
    std::string random_id;
    for(int n = 0; n < 10; ++n) {
        int random_integer = distr(gen); // generate numbers
        random_id = random_id + std::to_string(random_integer); // append 10 different numbers to generate a unique id
    }    
    json["id"] = random_id;
    // Dump JSON to string
    request = json.dump(4);////json.dump();
    #endif
    #ifdef NEROSHOP_DEBUG
    std::cout << "\"" << sql << "\" has been translated to: \n" << request << std::endl;
    #endif    
    return request;
}

std::string neroshop::rpc::translate(const std::string& sql, const std::vector<std::string>& args) {
    std::string request = "";
    #if defined(NEROSHOP_USE_QT)
    QJsonObject request_object; // JSON-RPC Request object
    
    request_object.insert(QString("jsonrpc"), QJsonValue("2.0"));
    request_object.insert(QString("method"), QJsonValue("query"));
    QJsonObject params_object;
    params_object.insert(QString("sql"), QJsonValue(QString::fromStdString(sql))); // eg. "SELECT * FROM products WHERE condition = $1;"
    params_object.insert(QString("count"), static_cast<int>(args.size()));
    for(int index = 0; index < args.size(); index++) {
        std::string arg_key = /*"" + */std::to_string(index + 1);
        std::string arg_value = args[index];
        params_object.insert(QString::fromStdString(arg_key), QJsonValue(QString::fromStdString(arg_value)));
        std::cout << "" << (index + 1) << "= " << arg_value << "\n";
    }
    request_object.insert(QString("params"), QJsonValue(params_object));
    // Generate random number for id (id can be either a string or an integer or null which is not recommended)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(1, 9);
    std::string random_id;
    for(int n = 0; n < 10; ++n) {
        int random_integer = distr(gen);
        random_id = random_id + std::to_string(random_integer);
    }
    request_object.insert(QString("id"), QJsonValue(QString::fromStdString(random_id)));
    // Convert JSON to string
    QJsonDocument json_doc(request_object);
    QString json_str = json_doc.toJson();
    request = json_str.toStdString();
    #else
    nlohmann::json json;
    json["jsonrpc"] = "2.0";
    json["method"] = "query";////get_method(sql);
    json["params"]["sql"] = sql;
    json["params"]["count"] = args.size();
    for(int index = 0; index < args.size(); index++) {
        std::string arg_key = std::to_string(index + 1);
        std::string arg_value = args[index];
        json["params"][arg_key] = arg_value;
    }
    // Generate random number for id (id can be either a string or an integer or null which is not recommended)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(1, 9);
    std::string random_id;
    for(int n = 0; n < 10; ++n) {
        int random_integer = distr(gen);
        random_id = random_id + std::to_string(random_integer);
    }    
    json["id"] = random_id;
    // Dump JSON to string
    request = json.dump(4);////json.dump();    
    #endif
    #ifdef NEROSHOP_DEBUG
    std::cout << "\"" << sql << "\" has been translated to: \n" << request << std::endl;
    #endif        
    return request;
}

void neroshop::rpc::request(const std::string& json) { // TODO: this function should return a json_rpc response from the server
    // Get the json which is basically a translated sqlite query or a translate method string + args
    // Passing the data onto the server
    ////zmq_send (requester, request.c_str(), request.size(), 0);  
    // The server will then execute the data
    // Lastly, the server will return any results
} // Usage: neroshop::rpc::request(neroshop::rpc::translate("SELECT * FROM users;"));

void request_batch(const std::vector<std::string>& json_batch) {
}

void neroshop::rpc::respond(const std::string& json) {
    std::string response = "";   
    #if defined(NEROSHOP_USE_QT)
    #endif
}

void neroshop::rpc::respond_batch(const std::vector<std::string>& json_batch) {
}

std::string neroshop::rpc::get_method(const std::string& sql) {
	std::string first_word = sql.substr(0, sql.find_first_of(" "));
    if(is_valid_method(first_word)) return first_word;
    return "";
}

bool neroshop::rpc::is_valid_method(const std::string& method) {
    return ((std::find(sql_methods.begin(), sql_methods.end(), method) != sql_methods.end()) || (std::find(methods.begin(), methods.end(), method) != methods.end()));
}
