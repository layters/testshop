#include <iostream>
#include <string>

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
        {"get         ", "Get the value for a key from DHT network"},
        {"status      ", "Get network status from local daemon server"}
        //,{"", ""}
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
    neroshop::Client * client = neroshop::Client::get_main_client();
	int port = NEROSHOP_IPC_DEFAULT_PORT;
	std::string ip = "localhost"; // 0.0.0.0 means anyone can connect to your server
	if(!client->connect(port, ip)) {
	    std::cout << "Please launch neromon first\n";
	    exit(0);
	}
    //-------------------------------------------------------
    neroshop::load_nodes_from_memory();

    std::vector<std::string> networks = {"mainnet", "testnet", "stagenet"};

    std::string network_type = Wallet::get_network_type_as_string();
    if (std::find(networks.begin(), networks.end(), network_type) == networks.end()) {
        neroshop::print("\033[1;91mnetwork_type \"" + network_type + "\" is not valid");
        return 1;
    }
    std::vector<std::string> monero_nodes = Script::get_table_string(lua_state, "monero.nodes." + network_type);
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
            std::cout << "\033[0;93m" << NEROSHOP_VERSION_FULL << "\033[0m" << std::endl;
        }         
        else if(neroshop::string::starts_with(command, "query")) {
            auto arg_count = neroshop::string::split(command, " ").size();
            if(arg_count == 1) std::cout << "\033[91mexpected arguments after 'query'\033[0m\n";
            if(arg_count > 1) { 
                std::size_t arg_pos = command.find_first_of(" ");
                std::string sql = neroshop::string::trim_left(command.substr(arg_pos + 1));////trim_left(command.substr(std::string("query").length() + 1)); // <- this works too
                assert(neroshop::string::starts_with(sql, "SELECT", false) && "Only SELECT queries are allowed"); // since the ability to run sql commands gives too much power to the user to alter the database anyhow, limit queries to select statements only
                std::string json = neroshop::rpc::json::translate(sql);
                // No need to call RPC server to access RPC functions when it can be done directly.
                std::cout << neroshop::rpc::json::process(json) << "\n";
                // Usage: query SELECT * FROM mappings;
            }
        }
        else if(command == "status") {
            if (client->is_connected()) {
                std::string response;
                client->get("status", response);
            
                try {
                    nlohmann::json json = nlohmann::json::parse(response);
                    std::cout << json.dump(4) << "\033[0m\n";
                } catch (const nlohmann::detail::parse_error& e) {
                    std::cerr << "Parse error\n";
                }
            }
        }
        else if(command == "curl_version") {
            curl_version_info_data * curl_version = curl_version_info(CURLVERSION_NOW);
            std::string curl_version_str = std::to_string((curl_version->version_num >> 16) & 0xff) + 
                "." + std::to_string((curl_version->version_num >> 8) & 0xff) + 
                "." + std::to_string(curl_version->version_num & 0xff);
            std::cout << "libcurl version " << curl_version_str << std::endl;
        }
        else if(neroshop::string::starts_with(command, "get")) {
            auto arg_count = neroshop::string::split(command, " ").size();
            if(arg_count == 1) std::cout << "\033[91mexpected argument after 'get'\033[0m\n";
            if(arg_count > 1) { 
                std::size_t arg_pos = command.find_first_of(" ");
                std::string key = neroshop::string::trim(command.substr(arg_pos + 1));
                
                if (client->is_connected()) {
                    std::string response;
                    client->get(key, response);
                    
                    try {
                        nlohmann::json json = nlohmann::json::parse(response);
                        std::cout << (json.contains("error") ? "\033[91m" : "\033[32m") << json.dump(4) << "\033[0m\n";
                    } catch (const nlohmann::detail::parse_error& e) {
                        std::cerr << "Parse error\n";
                    }
                }
            }
        }     
        else if(command == "register") {
            User user;
            //user.set_name("Jack");
            //user.set_public_key("<public_key>");
            //user.set_id("<monero_addr>");
            auto data = neroshop::Serializer::serialize(user);
            std::string key = data.first;
            std::string value = data.second;
    
            // Send put and receive response
            std::string put_response;
            client->put(key, value, put_response);
            std::cout << "Received response: " << put_response << "\n";
        }
        /*else if(command == "") {
        }*/        
        else if(command == "exit") {
            // close the connection
            client->disconnect();
            break;
        }
        else {
            std::cerr << std::string("\033[0;91mUnrecognized command: \033[1;37m") << command << "\033[0m" << std::endl;  
        }
        linenoiseFree(line); // Or just free(line) if you use libc malloc.
    }
    return 0;
}
