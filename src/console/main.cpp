#include <iostream>
#include <string>
// neroshop
#include "../neroshop.hpp"
using namespace neroshop;
// linenoise
#include <linenoise.h>

lua_State * neroshop::lua_state = luaL_newstate(); // lua_state should be initialized by default

int main() {
    
    if(!neroshop::create_config()) { 
        neroshop::load_config();
    }

    std::vector<std::string> networks = {"mainnet", "stagenet", "testnet"};

    std::string network_type = Script::get_string(lua_state, "neroshop.daemon.network_type");
    if (std::find(networks.begin(), networks.end(), network_type) == networks.end()) {
        neroshop::print("\033[1;91mnetwork_type \"" + network_type + "\" is not valid");
        return 1;
    }

    std::vector<std::string> nodes_list = Script::get_table_string(neroshop::get_lua_state(), "neroshop.nodes."+network_type);
    
    if(nodes_list.empty()) {
        std::cout << "failed to get nodes in the config file\nCheck your config file in ~/.config/neroshop" << std::endl;
    }
    //-------------------------
    // todo: bind command names to functions
    char * line = NULL;
    while((line = linenoise("neroshop-console> ")) != NULL) {
        // Do something with the string
        std::string command { line };
        //std::cout << "You wrote: " << command.c_str() << " (" << command.size() << ")\n";
        linenoiseHistoryAdd(line); // Add to the history.
        linenoiseHistorySave("history.txt"); // Save the history on disk.    
        // commands        
        // without the command == "", single letters like 'e' will call exit for some reason
        if(command == "help") {
            std::cout << "\033[1;37mAvailable commands:\n\n help  Display list of available commands\n\n exit  Exit CLI\n\n" << "\033[0m\n";
        }  
        else if(command == "monero_nodes") {
            for(std::string n : nodes_list) {
                std::cout << "\033[1;36m" << n << "\033[0m" << std::endl;
            }        
        }            
        else if(command == "version") {
            std::cout << "\033[0;93m" << "neroshop v" << APP_VERSION << "\033[0m" << std::endl;
        }         
        else if(command == "exit") {
            break;//exit(0);
        }  
        // By default, pressing "Enter" on an empty string will execute the first call to strncmp for some reason
        else if(!strncmp(line, "\0", command.length())) { // Empty string
            std::cout << "Available commands:\n\n help  Display list of available commands\n\n exit  Exit CLI\n\n";
        }
        else {
            std::cerr << std::string("\033[1;91mUnreconized command: \033[1;37m") << line << "\033[0m" << std::endl;
        }                   
        linenoiseFree(line); // Or just free(line) if you use libc malloc.
    }
        /*else if(!strncmp(line, "", command.size()) && command == "") {
        
        }*/    

    //-------------------------------------
    /*else if(shellinput == "register") { // create a new wallet
        
    }    
    */  
    /*else if(shellinput == "auth") { // auth or access = synonym for login
       std::cout << "1. seed (mnemonic)\n2. keys\n3. open wallet file";
       unsigned int auth_options;
       std::cin >> auth_options;
       if(auth_options == 1) {
           
       }
       if(auth_options == 2) {
           
       }
    }*/         
    /*else if(shellinput == "auth_with_seed") { // restore wallet from seed
        
    }    
    else if(shellinput == "auth_with_keys") { // restore wallet from keys
        
    }
    else if(shellinput == "auth_with_hw") { // restore/create wallet from hw
        
    }            
    */        
    /*else if(shellinput == "") {
        
    }*/        
    //-------------------------    
    //neroshop::Server server;
    //server.bind("exit", [](void) { ::system("exit"); });    
    
    //client.call("exit" /*args ...*/);
    //-------------------------    
    return 0;
}
