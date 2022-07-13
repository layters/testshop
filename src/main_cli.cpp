#include <iostream>
#include <string>
// neroshop
#include "../include/neroshop.hpp"
using namespace neroshop;
// dokun-ui
#include <build.hpp>
#include DOKUN_HEADER
using namespace dokun;

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

    while (true) {
	std::string shellinput;
        std::cout << "neroshop>";
        std::cin >> shellinput;

	if (shellinput == "help") {std::cout << "Available commands:\n\n help  Display list of available commands\n\n exit  Exit CLI\n\n";}
    else if (shellinput == "monero_nodes") {
        for (std::string n : nodes_list) {
            std::cout << n << std::endl;
        }
    }
    else if (shellinput == "exit") {break;}
    }
    return 0;
}
