#include <iostream>
#include <string>

#include "../core/protocol/transport/zmq_client.hpp"

#include "../neroshop.hpp"
using namespace neroshop;
namespace neroshop_tools = neroshop::tools;

#include <linenoise.h>

static void print_commands() {
    std::map<std::string, std::string> commands { // std::map sorts command names in alphabetical order
        {"help        ", "Display list of available commands"},
        {"exit        ", "Exit CLI"},
        {"version     ", "Show version"},
        {"monero_nodes", "Display a list of monero nodes"}, 
        {"query       ", "Execute an SQLite query"}, 
        {"curl_version", "Show libcurl version"}, 
        {"download_tor", "Download Tor"}, 
        {"send        ", "Send a test message to the local daemon IPC server"}
        /*, 
        {"", ""}*/
    };
    std::cout << "Usage: " << "[COMMAND] ...\n\n";
    for (auto const& [key, value] : commands) {
        std::cout << "  " << key << "  " << value << "\n";
    }
    std::cout << "\n";
}

int main(int argc, char** argv) {
    //-------------------------------------------------------
    // Connect to daemon server (daemon must be launched first)
    #if !defined(NEROSHOP_USE_LIBZMQ)
    neroshop::Client * client = neroshop::Client::get_main_client();
	int port = DEFAULT_TCP_PORT;
	std::string ip = "localhost"; // 0.0.0.0 means anyone can connect to your server
	if(!client->connect(port, ip)) {
	    std::cout << "Please launch neromon first\n";
	    exit(0);
	}
	#else
	neroshop::ZmqClient client("tcp://localhost:5555");
	#endif
    //-------------------------------------------------------
    // nodes.lua
    if(!neroshop::create_config()) { 
        neroshop::load_config();
    }

    std::vector<std::string> networks = {"mainnet", "stagenet", "testnet"};

    std::string network_type = Script::get_string(lua_state, "monero.network_type");
    if (std::find(networks.begin(), networks.end(), network_type) == networks.end()) {
        neroshop::print("\033[1;91mnetwork_type \"" + network_type + "\" is not valid");
        return 1;
    }
    std::vector<std::string> monero_nodes = Script::get_table_string(lua_state, "monero.nodes." + network_type);
    
    if(monero_nodes.empty()) {
        std::cout << "failed to get nodes in the config file\nCheck your config file in ~/.config/neroshop" << std::endl;
    }
    //-------------------------------------------------------
    char * line = NULL;
    while((line = linenoise("neroshop-console> ")) != NULL) {
        std::string command { line };

        linenoiseHistoryLoad("history.txt");
        linenoiseHistoryAdd(line); // Add to the history.
        linenoiseHistorySave("history.txt"); // Save the history on disk.

        if(command == "help" || !strncmp(line, "\0", command.length())) { // By default, pressing "Enter" on an empty string will execute this code
            print_commands();
        }  
        else if(command == "monero_nodes") {
            for(std::string nodes : monero_nodes) {
                std::cout << "\033[1;36m" << nodes << "\033[0m" << std::endl;
            }        
        }            
        else if(command == "version") {
            std::cout << "\033[0;93m" << "neroshop v" << NEROSHOP_VERSION << "\033[0m" << std::endl;
        }         
        else if(neroshop::string::starts_with(command, "query")) {
            auto arg_count = neroshop::string::split(command, " ").size();
            if(arg_count == 1) neroshop::print("expected arguments after 'query'", 1);
            if(arg_count > 1) { 
                 std::size_t arg_pos = command.find_first_of(" ");
                 std::string sql = neroshop::string::trim_left(command.substr(arg_pos + 1));////trim_left(command.substr(std::string("query").length() + 1)); // <- this works too
                 assert(neroshop::string::starts_with(sql, "SELECT", false) && "Only SELECT queries are allowed"); // since the ability to run sql commands gives too much power to the user to alter the database anyhow, limit queries to select statements only
                 std::string json = neroshop::rpc::json::translate(sql);
                 // No need to call RPC server to access RPC functions when it can be done directly.
                 std::cout << neroshop::rpc::json::process(json) << "\n";
                 // Usage: query SELECT * FROM users;
            }
        }
        else if(command == "curl_version") {
            curl_version_info_data * curl_version = curl_version_info(CURLVERSION_NOW);
            std::string curl_version_str = std::to_string((curl_version->version_num >> 16) & 0xff) + 
                "." + std::to_string((curl_version->version_num >> 8) & 0xff) + 
                "." + std::to_string(curl_version->version_num & 0xff);
            std::cout << "libcurl version " << curl_version_str << std::endl;
        }
        else if(command == "download_tor") {
            std::packaged_task<void(void)> download_task([&]() -> void {
                neroshop_tools::downloader::download_tor();
            });
            std::thread { std::move(download_task) }.detach();
        }
        else if(command == "send") { // This is only a test command
            #if !defined(NEROSHOP_USE_LIBZMQ)
            if (client->is_connected()) {
            nlohmann::json arguments_obj = {
                {"id", ""},
            };//nlohmann::json nested_array = {"item1", "item2", "item3"};
            nlohmann::json j = {
                {"version", std::string(NEROSHOP_VERSION)},
                {"query", "ping"},
                {"args", arguments_obj},
                {"tid", 123},
            };
            // send packed data to POSIX server
            std::vector<uint8_t> packed = nlohmann::json::to_msgpack(j);
            client->send(packed);//client.send("Hello, POSIX Server!");
            
            try {
            std::vector<uint8_t> response;//std::string response;
            client->receive(response);
            // Deserialize the response from MessagePack to a JSON object
            nlohmann::json j2 = nlohmann::json::from_msgpack(response);
            std::cout << "Received response: " << j2.dump() << std::endl;
            } catch (const nlohmann::detail::parse_error& e) {
                std::cerr << "An error occurred: " << "Server was disconnected" << std::endl;//std::cout << "Failed to parse server response: " << e.what() << std::endl;
            }
            
            } else {
                std::cout << "Failed to establish connection to server\n";
            }
            //------------------------------------------------------------
            #else
            nlohmann::json j = {
                {"message", "Hello, ZMQ Server!"},
                {"id", 123}
            };
            // send packed data to ZMQ server
            std::vector<uint8_t> packed = nlohmann::json::to_msgpack(j);
            client.send(packed);//client.send("Hello, ZMQ Server!");
            
            std::vector<uint8_t> response;//std::string response;
            client.receive(response);
            // Deserialize the response from MessagePack to a JSON object
            nlohmann::json j2 = nlohmann::json::from_msgpack(response);
            std::cout << "Received response: " << j2.dump() << std::endl;//std::cout << "Received response: " << response << std::endl;
            #endif
        }        
        else if(command == "put") {
            if (client->is_connected()) {
                nlohmann::json arguments_obj = {
                    {"id", "my_node_id"},
                    {"key", "name"},
                    {"value", "Jack"},
                };
                nlohmann::json j = {
                    {"version", std::string(NEROSHOP_VERSION)},
                    {"query", "put"},
                    {"args", arguments_obj},
                    {"tid", 123},
                };
                // send packed data to POSIX server
                std::vector<uint8_t> packed = nlohmann::json::to_msgpack(j);
                client->send(packed);//client.send("Hello, POSIX Server!");
            
                try {
                std::vector<uint8_t> response;//std::string response;
                client->receive(response);
                // Deserialize the response from MessagePack to a JSON object
                nlohmann::json j2 = nlohmann::json::from_msgpack(response);
                std::cout << "Received response: " << j2.dump() << std::endl;
                } catch (const nlohmann::detail::parse_error& e) {
                    std::cerr << "An error occurred: " << "Server was disconnected" << std::endl;//std::cout << "Failed to parse server response: " << e.what() << std::endl;
                }
            }       
        } 
        else if(command == "get") {
            if (client->is_connected()) {
                nlohmann::json arguments_obj = {
                    {"id", "my_node_id"},
                    {"key", "name"},
                };
                nlohmann::json j = {
                    {"version", std::string(NEROSHOP_VERSION)},
                    {"query", "get"},
                    {"args", arguments_obj},
                    {"tid", 123},
                };
                // send packed data to POSIX server
                std::vector<uint8_t> packed = nlohmann::json::to_msgpack(j);
                client->send(packed);//client.send("Hello, POSIX Server!");
            
                try {
                std::vector<uint8_t> response;//std::string response;
                client->receive(response);
                // Deserialize the response from MessagePack to a JSON object
                nlohmann::json j2 = nlohmann::json::from_msgpack(response);
                std::cout << "Received response: " << j2.dump() << std::endl;
                } catch (const nlohmann::detail::parse_error& e) {
                    std::cerr << "An error occurred: " << "Server was disconnected" << std::endl;//std::cout << "Failed to parse server response: " << e.what() << std::endl;
                }
            }
        }        
        /*else if(command == "") {
        }*/        
        else if(command == "exit") {
            #if !defined(NEROSHOP_USE_LIBZMQ)
            // close the connection
            client->close();
            #endif
            break;
        }
        else {
            std::cerr << std::string("\033[0;91mUnreconized command: \033[1;37m") << command << "\033[0m" << std::endl;  
        }
        linenoiseFree(line); // Or just free(line) if you use libc malloc.
    }
    return 0;
}
