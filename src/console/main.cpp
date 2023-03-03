#include <iostream>
#include <string>

#include "../neroshop.hpp"
using namespace neroshop;

#include <linenoise.h>

int main(int argc, char** argv) {
    // TODO: map command names to functions
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
                 std::string json = neroshop::rpc::translate(sql);
                 neroshop::rpc::process(json);//neroshop::rpc::request(json);
                 // Usage: query UPDATE users SET name = "dude" WHERE name = "jack"
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
