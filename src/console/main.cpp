#include <iostream>
#include <string>

#include "../neroshop.hpp"
using namespace neroshop;

#include <linenoise.h>

int main(int argc, char** argv) {
    // Connect to daemon server (daemon must be launched first)
    #ifndef NEROSHOP_DEBUG
    neroshop::Client * client = neroshop::Client::get_main_client();
	int port = 40441;
	std::string ip = "0.0.0.0";//"localhost"; // 0.0.0.0 means anyone can connect to your server
	if(!client->connect(port, ip)) {
	    std::cout << "Please launch neromon first\n";
	    exit(0);
	}
	#endif
    //-------------------------------------------------------
    if(!neroshop::create_config()) { 
        neroshop::load_config();
    }

    std::vector<std::string> networks = {"mainnet", "stagenet", "testnet"};

    std::string network_type = Script::get_string(lua_state, "neroshop.monero.daemon.network_type");
    if (std::find(networks.begin(), networks.end(), network_type) == networks.end()) {
        neroshop::print("\033[1;91mnetwork_type \"" + network_type + "\" is not valid");
        return 1;
    }
    std::vector<std::string> monero_nodes = Script::get_table_string(lua_state, "neroshop.monero.nodes." + network_type);
    
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
            std::cout << "\033[1;37mAvailable commands:\n\n help  Display list of available commands\n\n exit  Exit CLI\n\n" << "\033[0m\n";
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
                 std::string json = neroshop::rpc::translate(sql);
                 #ifdef NEROSHOP_DEBUG
                 neroshop::rpc::process(json);
                 #else
                 client->write(json); // write request to server
                 std::string response_object = client->read(); // read response from server
                 std::cout << "Server response: " << response_object << "\n";
                 #endif
                 // Usage: query SELECT * FROM users;
            }
        }
        else if(command == "exit") {
            break;
        }
        else {
            std::cerr << std::string("\033[1;91mUnreconized command: \033[1;37m") << line << "\033[0m" << std::endl;  
        }
        linenoiseFree(line); // Or just free(line) if you use libc malloc.
    }
    return 0;
}
