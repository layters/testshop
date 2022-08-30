#pragma once

#ifndef CONFIG_HPP_NEROSHOP
#define CONFIG_HPP_NEROSHOP

#define LUA_TAG "\033[1;34m[lua]:\033[0m "
// configuration dir(s)
// windows
#if defined(_WIN32)
#define NEROSHOP_CONFIG_PATH "C:/ProgramData/neroshop"//"C:/Users/<USER>/AppData/Local/<APPNAME>", "C:/ProgramData/<APPNAME>" (AppConfigLocation)
#endif
// linux
#if defined(__linux__) && !defined(__ANDROID__)
#define NEROSHOP_CONFIG_PATH "/home/" + neroshop::device::get_user() + "/.config/neroshop"//"~/.config/<APPNAME>", "/etc/xdg/<APPNAME>" (AppConfigLocation)
#endif
// macos
#if defined(__APPLE__) && defined(__MACH__)
#define NEROSHOP_CONFIG_PATH "~/Library/Preferences/neroshop"//"~/Library/Preferences/<APPNAME>" (AppConfigLocation)
#endif
// android
#if defined(__ANDROID__)
//"<APPROOT>/files/settings" (AppConfigLocation)
#endif
// iOS
#if defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
//"<APPROOT>/Library/Preferences/<APPNAME>" (AppConfigLocation)
#endif
#endif

#define NEROSHOP_CONFIG_FILE "config.lua"

#include "script.hpp"//<script.hpp>
#include <filesystem>

#include "util.hpp" // neroshop::device::get_user
#include "debug.hpp"

namespace neroshop {
    extern lua_State * lua_state;// (luaL_newstate());//(nullptr);// extern is for declaration in .h and then defined in a .cpp file
    //////////////////////////////
	static bool load_config() {
        std::string user = neroshop::device::get_user();
        // "/home/<user>/.config/neroshop"
        std::string neroshop_config_path = NEROSHOP_CONFIG_PATH;//"/home/" + user + "/.config/neroshop";
        // "/home/<user>/.config/neroshop/config.lua"
        std::string neroshop_config_name = neroshop_config_path + "/" + NEROSHOP_CONFIG_FILE;
        Script script;
        if(!script.load(lua_state, neroshop_config_name)) {return false;}
        neroshop::print("\033[1;94mloaded script \"" + script.get_file() + "\"");
        return true; // default return-value
	}
	//////////////////////////////
	static bool create_config() {
        std::string user = neroshop::device::get_user();
        std::string text = "-- config.lua\n"
"localhost = \"127.0.0.1\";\n"
"neroshop = {};\n"
"neroshop[\"currency\"] = \"usd\";\n"//\n"
"neroshop[\"window\"] = {};\n"
"--neroshop[\"window\"][\"x\"] = 200;\n"
"--neroshop[\"window\"][\"y\"] = 200;\n"
"neroshop[\"window\"][\"width\"] = 1280;\n"
"neroshop[\"window\"][\"height\"] = 720;\n"
"neroshop[\"window\"][\"mode\"] = 0; --0 (window_mode) or 1 (fullscreen)\n"//\n"
"neroshop[\"daemon\"] = {};\n" // maybe change name from daemon to node or nah?
"neroshop[\"daemon\"][\"network_type\"] = \"stagenet\"; --\"mainnet\", \"stagenet\", or \"testnet\"\n"
"neroshop[\"daemon\"][\"ip\"] = localhost; -- set to \"0.0.0.0\" to allow other wallets to connect to your node\n"
"neroshop[\"daemon\"][\"port\"] = \"38081\"; --\"18081\" (mainnet), \"38081\" (stagenet), or 28081 (testnet)\n"
"neroshop[\"daemon\"][\"confirm_external_bind\"] = false; -- if true then it confirms that you want to allow connections from other wallets outside of this system, but only when ip is set to \"0.0.0.0\"\n"
"neroshop[\"daemon\"][\"restricted_rpc\"] = true;\n"
"neroshop[\"daemon\"][\"data_dir\"] = \"/home/<user>/.bitmonero\";\n"
"neroshop[\"daemon\"][\"remote\"] = false; -- set to true if the node that you want to connect to is a remote node\n"//\n"
"neroshop[\"wallet\"] = {};\n"
"neroshop[\"wallet\"][\"file\"] = \"\"; -- include \".keys\" extension\n" // path or file
"neroshop[\"wallet\"][\"restore_height\"] = \"2014-04-18\"; -- block height or date (YYYY-MM-DD)\n";
        // swap data_dir with user
    #if defined(__gnu_linux__) // works!    
        text = neroshop::string::swap_first_of(text, "/home/<user>/.bitmonero", ("/home/" + user + "/.bitmonero"));
    #endif    
        // "/home/<user>/.config/neroshop"
        std::string neroshop_config_path = NEROSHOP_CONFIG_PATH;//"/home/" + user + "/.config/neroshop"; // neroshop::device::get_user()
        // "/home/<user>/.config/neroshop/config.lua"
        std::string neroshop_config_name = neroshop_config_path + "/" + NEROSHOP_CONFIG_FILE;
        // if file already exists, no need to create it again
        if(std::filesystem::is_regular_file(neroshop_config_name)) return false; // false because it will not be created // if true then it will cause "PANIC: unprotected error in call to Lua API (attempt to index a nil value)" error
        // check if script works before saving
        if(luaL_dostring(lua_state, text.c_str()) != 0) {
		    neroshop::print(LUA_TAG "\033[0;91minvalid Lua code");
		    lua_error(lua_state);
		    return false; // exit function so it does not save text
	    }
        // if path does not exist
        if(!std::filesystem::is_directory(neroshop_config_path)) 
        {   // create the path
            neroshop::print("directory \"" + neroshop_config_path + "\" does not exist, but I will create it for you (^_^)", 2);
            if(!std::filesystem::create_directories(neroshop_config_path)) { neroshop::print("create_config error: failed to make the path. Sorry (ᵕ人ᵕ)! ...", 1); return false; }
            neroshop::print("\033[1;97;49mcreated path \"" + neroshop_config_path + "\"");
        }
        // if path exists, but the file is missing or deleted
        if(!std::filesystem::is_regular_file(neroshop_config_name)) {
            // create config file (one time)
            std::ofstream cfg;
            cfg.open (neroshop_config_name, std::ios::out | std::ios::trunc);
            cfg << text << "\n"; // write to file
            cfg.close();
            neroshop::print("\033[1;97;49mcreated file \"" + neroshop_config_name + "\"\033[0m");  
        }
        return true;		
	}
	//////////////////////////////
	//static void edit_config(const std::string& old_str, const std::string& new_str); // not possible to edit lua files with my current knowledge
	//////////////////////////////
	static lua_State * get_lua_state() {
	    return lua_state;
	}
}

#endif
