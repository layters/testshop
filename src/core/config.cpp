#include "config.hpp"

lua_State * neroshop::lua_state(luaL_newstate());

bool neroshop::load_config() {
        std::string user = neroshop::device::get_user();
        // "/home/<user>/.config/neroshop"
        std::string configuration_path = NEROSHOP_DEFAULT_CONFIGURATION_PATH;
        std::string configuration_file = "settings.lua";
        // "/home/<user>/.config/neroshop/settings.lua"
        std::string neroshop_config_name = configuration_path + "/" + configuration_file;
        Script script;
        if(!script.load(lua_state, neroshop_config_name)) {
            return false;
        }
        neroshop::print("\033[1;94mloaded script \"" + script.get_file() + "\"");
        return true;
	}

bool neroshop::create_config() {
    std::string user = neroshop::device::get_user();
    std::string text = R"(-- settings.lua
localhost = "127.0.0.1"
neroshop = {
    generalsettings = {
        currency = "usd",
        application = {
            theme = {
                dark = false,
                name = "DefaultLight", --"DefaultDark", "DefaultLight", "PurpleDust",
            },        
            window = {
                width = 1280,
                height = 720,
                mode = 0, --0 (window_mode) or 1 (fullscreen)
            }
        }        
    },
    
    monero = {
        wallet = {
            file = "",
            restore_height = "2014-04-18" -- block height or date (YYYY-MM-DD)
        },
        daemon = {
            network_type = "stagenet",
            ip = "localhost", -- set to "0.0.0.0" to allow other wallets to connect to your node
            port = "38081", --"18081" (mainnet), "38081" (stagenet), or "28081" (testnet)
            confirm_external_bind = false, -- if true then it confirms that you want to allow connections from other wallets outside of this system, but only when ip is set to "0.0.0.0"
            restricted_rpc = true,
            data_dir = "/home/<user>/.bitmonero",
            remote = false -- set to true if the node that you want to connect to is a remote node
        },
        nodes = {
            mainnet = {
                "node.community.rino.io:18081",
                "node.sethforprivacy.com:18089",
                "node2.sethforprivacy.com:18089",
                "selsta1.featherwallet.net:18081",
                "selsta2.featherwallet.net:18081",
                "node.monerooutreach.org:18081",
                "node.majesticbank.is:18089",
                "node.majesticbank.su:18089",
                "xmr-node-eu.cakewallet.com:18081",
                "xmr-node-usa-east.cakewallet.com:18081",
                "canada.node.xmr.pm:18089",
                "singapore.node.xmr.pm:18089",
                "nodes.hashvault.pro:18081",
                "node.supportxmr.com:18081",
                "node.xmr.ru:18081"
            },
            stagenet = {
                "super.fast.node.xmr.pm:38089",
                "stagenet.community.rino.io:38081"
            },
            testnet = {
                "testnet.community.rino.io:28081"
            }
        }
    }
})";
        // swap data_dir with user
    #if defined(__gnu_linux__) // works!    
        text = neroshop::string::swap_first_of(text, "/home/<user>/.bitmonero", ("/home/" + user + "/.bitmonero"));
    #endif    
        // "/home/<user>/.config/neroshop"
        std::string configuration_path = NEROSHOP_DEFAULT_CONFIGURATION_PATH;//"/home/" + user + "/.config/neroshop";
        std::string configuration_file = "settings.lua";
        // "/home/<user>/.config/neroshop/config.lua"
        std::string neroshop_config_name = configuration_path + "/" + configuration_file;
        // if file already exists, no need to create it again
        if(std::filesystem::is_regular_file(neroshop_config_name)) return false; // false because it will not be created // if true then it will cause "PANIC: unprotected error in call to Lua API (attempt to index a nil value)" error
        // check if script works before saving
        if(luaL_dostring(lua_state, text.c_str()) != 0) {
		    neroshop::print(LUA_TAG "\033[0;91minvalid Lua code");
		    lua_error(lua_state);
		    return false; // exit function so it does not save text
	    }
        // if path does not exist
        if(!std::filesystem::is_directory(configuration_path)) 
        {   // create the path
            neroshop::print("directory \"" + configuration_path + "\" does not exist, but I will create it for you (^_^)", 2);
            if(!std::filesystem::create_directories(configuration_path)) { neroshop::print("create_config error: failed to make the path. Sorry (ᵕ人ᵕ)! ...", 1); return false; }
            neroshop::print("\033[1;97;49mcreated path \"" + configuration_path + "\"");
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
	//void neroshop::edit_config(const std::string& old_str, const std::string& new_str); // not possible to edit lua files with my current knowledge

extern bool neroshop::open_config() {
    if(!neroshop::create_config()) { 
        if(!neroshop::load_config()) {
            neroshop::print("Failed to load configuration file", 1);
            return false;
        }
    }
    return true;
}

bool neroshop::create_configuration_file() {
    return neroshop::create_config();
}

bool neroshop::load_configuration_file() {
    return neroshop::load_config();
}

bool neroshop::open_configuration_file() {
    return neroshop::open_config();
}

lua_State * neroshop::get_lua_state() {
	return lua_state;
}
