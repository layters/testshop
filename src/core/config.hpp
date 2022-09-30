#pragma once

#ifndef CONFIG_HPP_NEROSHOP
#define CONFIG_HPP_NEROSHOP

#if defined(NEROSHOP_USE_QT)
#include <QStandardPaths>
#define NEROSHOP_DEFAULT_CONFIGURATION_PATH QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
#endif

#if !defined(NEROSHOP_USE_QT)
#if defined(_WIN32)
#define NEROSHOP_DEFAULT_CONFIGURATION_PATH "C:/Users/" + neroshop::device::get_user() + "/AppData/Local/neroshop"
#define NEROSHOP_DEFAULT_CONFIGURATION_PATH_ALT "C:/ProgramData/neroshop"
#endif
#if defined(__linux__) && !defined(__ANDROID__)
#define NEROSHOP_DEFAULT_CONFIGURATION_PATH "/home/" + neroshop::device::get_user() + "/.config/neroshop"
#define NEROSHOP_DEFAULT_CONFIGURATION_PATH_ALT "/etc/xdg/neroshop"
#endif
#if defined(__APPLE__) && defined(__MACH__)
//#define NEROSHOP_DEFAULT_CONFIGURATION_PATH "~/Library/Preferences/neroshop"
#endif
#if defined(__ANDROID__)
//#define NEROSHOP_DEFAULT_CONFIGURATION_PATH "<APPROOT>/files/settings"
#endif
#if defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
//#define NEROSHOP_DEFAULT_CONFIGURATION_PATH "<APPROOT>/Library/Preferences/neroshop"
#endif
#endif
#endif // endif !defined(NEROSHOP_USE_QT)

#define NEROSHOP_CONFIGURATION_NAME "settings"
#define NEROSHOP_CONFIGURATION_EXTENSION "lua"
#define NEROSHOP_CONFIGURATION_FILE NEROSHOP_CONFIGURATION_NAME "." NEROSHOP_CONFIGURATION_EXTENSION

#define LUA_TAG "\033[1;34m[lua]:\033[0m "

#include "script.hpp"//<script.hpp>
#include <filesystem>

#include "util.hpp" // neroshop::device::get_user
#include "debug.hpp"

namespace neroshop {
    extern lua_State * lua_state;
	extern bool create_config();
	extern bool load_config();
	extern bool open_config(); // create_config + load_config
	//static void edit_config(const std::string& old_str, const std::string& new_str); // not sure how to edit lua files with my current knowledge
	// alternative function names
    extern bool create_configuration_file();
	extern bool load_configuration_file();
	extern bool open_configuration_file(); // create_config + load_config
		
	extern lua_State * get_lua_state();
}

#endif
