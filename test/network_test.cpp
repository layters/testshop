
#include <iostream>
// neroshop
#include "../include/client.hpp"
#include "../include/server.hpp"
using namespace neroshop;
// dokun-ui
#include <dokun_ui.hpp>
using namespace dokun;

namespace neroshop {
lua_State * lua_state = luaL_newstate(); // lua_state should be initialized by default
}

Process * server_process;
//////////////////////////////////////
void launch_neromon() {
    // on launching neroshop, start the neromon process, if it has not yet been started    
    int neromon = Process::get_process_by_name("neromon");
    if(neromon != -1) {
        neroshop::print("neromon is already running in the background", 4);
        return;
    }
    server_process = new Process(); // don't forget to delete this!
    server_process->create("./neromon", "");
    // show all processes
    ////Process::show_processes();
}
//////////////////////////////////////
void wait_neromon() {
    ::sleep(2);
}
//////////////////////////////////////
int main() {
    // neromon
    launch_neromon();
    // wait for neromon to finish launching
    wait_neromon();
    // connect client (neroshop) to neromon
    Client * client = Client::get_main_client();
	int client_port = 1234;
	std::string client_ip = "0.0.0.0";//"localhost";//0.0.0.0 means anyone can connect to your server
	if(!client->connect(client_port, client_ip)) {
	    // free process
	    delete server_process; // kills process
	    server_process = nullptr;
	    // exit application
	    exit(0);
	} else std::cout << client->read() << std::endl; // read from server once
	// on browser, go to: https://127.0.0.1:1234/
	// or https://localhost:1234/
	// brave (chromium): ERR_SSL_PROTOCOL_ERROR
	// firefox: SSL_ERROR_RX_RECORD_TOO_LONG    
	while(1) {}
    return 0;
}
