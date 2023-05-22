#pragma once

#ifndef SETTINGS_HPP_NEROSHOP
#define SETTINGS_HPP_NEROSHOP

#include "../neroshop_config.hpp"

#include "script.hpp" // lua_State

#define LUA_TAG "\033[1;34m[lua]:\033[0m "

namespace neroshop {

//namespace settings {
    extern lua_State * lua_state;
	extern bool export_lua();
	extern bool load_lua();
	extern bool open_lua(); // export_lua + load_lua
	extern bool load_nodes_from_memory();
	extern lua_State * get_lua_state();
	
	extern bool create_json();
	extern std::string load_json();
	extern bool open_json(std::string& out);
	extern void modify_json(const std::string& settings);
//}
	
}

#endif
