#pragma once

#ifndef SCRIPT_HPP_NEROSHOP
#define SCRIPT_HPP_NEROSHOP

#include <lua.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
#include <filesystem> // std::filesystem

namespace neroshop {

class Script {
	public:
	    Script(void);                                              static int new_(lua_State *L);
		Script(lua_State *L, const std::string& file_name); // use only in C++
		~Script(void); 
        // normal			
		bool load(lua_State * L, const std::string& file_name);    static int load(lua_State *L);	
		static bool load_script(lua_State *L, const std::string& file_name); static int load_script(lua_State *L); // loads a script
		static bool load_directory(lua_State * L, const std::string& directory);  static int load_directory(lua_State *L);
		static bool execute(lua_State * L, const std::string& command);          static int execute(lua_State *L); // executes a script command
		static void unload(std::string file_name);
		void generate(const char * script = NULL);                 static int generate(lua_State *L);  // generates a main.lua program (for GUI version of the engine)
		void write(std::string code);                              static int write(lua_State *L);
		void read(int line);                                       static int read(lua_State *L); 
		void copy(std::string file_name);                          static int copy(lua_State *L);  // copies the content of another script
		void copy(Script& script); // copy the contents of another script
		static bool save(lua_State *L, const std::string& table); static int save(lua_State *L);// saves a table's data to a text file
		void clean_stack(); // void clean() { int n = lua_gettop(L); lua_pop(L, n); }
		static void save_cache();
		void write_to_table(lua_State *L, void * object, const std::string& _class);
		static bool call(lua_State * L, const std::string& function, int returns = 0); // calls either a C++ function or Lua function
		// setters
		// getters
		static void get_table            (lua_State *L, const std::string& table); // pushes a global to the stack
		static std::string get_string    (lua_State * L, const std::string& key); // e.g script->get("player.name");  => "Jack"
		static int get_boolean           (lua_State * L, const std::string& key);
		static double get_number         (lua_State * L, const std::string& key);
		static void * get_userdata       (lua_State * L, const std::string& key);
		static lua_CFunction get_function(lua_State * L, const std::string& key); // returns a C++ function
		static lua_State * get_thread    (lua_State * L, const std::string& key); // a thread is represented as a lua_State
		static void * get_pointer        (lua_State * L, const std::string& key);
		static std::string get_type      (lua_State * L, const std::string& object);
		// table + arrays
		static std::vector<std::string>  get_table_string(lua_State *L, const std::string& table);
		static std::vector<int>          get_table_integer(lua_State *L, const std::string& table);
		static std::vector<float>        get_table_float(lua_State *L, const std::string& table);
		static std::vector<double>       get_table_double(lua_State *L, const std::string& table);
		static std::vector<double>       get_table_number(lua_State *L, const std::string& table);
		static std::vector<const void *> get_table_pointer(lua_State * L, const std::string& table); // can be a userdata, a table, a thread, or a function
		
		std::string get_file();                                    static int get_file(lua_State *L);
		std::string get_directory();                               static int get_directory(lua_State *L);
		std::string get_content();                                 static int get_content(lua_State *L); // returns content
		static int get_count();                                    static int get_count(lua_State *L); // returns number of scripts loaded
	    // boolean
		bool is_script();
		static bool is_script(std::string file_name);              static int is_script(lua_State *L); // checks if file has been loaded previously
	    // register globals, tables, functions, etc.
		static int table   (lua_State *L, std::string class_name); 
		static int function(lua_State *L, std::string table_name, std::string function_name, lua_CFunction function                );
		static int member  (lua_State *L, std::string table_name, std::string value_name, std::string value, int type = LUA_TSTRING);
		static int member  (lua_State *L, std::string table_name, std::string value_name, int value, int type = LUA_TNUMBER        );
	    static int global  (lua_State *L, std::string function_name, lua_CFunction function);
		static int global  (lua_State *L, std::string variable_name, int value             );
		static int global  (lua_State *L, std::string variable_name, std::string value     );		
		static int global  (lua_State *L, std::string variable_name, bool value            );	
		static int inherit (lua_State *L, std::string base_table, std::string sub_table    );
		static int attach  (lua_State *L, std::string base_table, std::string sub_table    );
		static int table_not_mt(lua_State *L, std::string class_name); // soon to be deprecated
	private:
	    static std::vector<std::string> cache;
		std::string content;
		std::string file;
};

}
/*
	Script * script = new Script(L, "example.lua");
	Sprite * sprite = *static_cast<Sprite **>(script->get_pointer(L, "player.udata"));
	sprite->draw();
	
	std::string file = script->get_string(L, "player.file_name");
	if(file.empty())
	{
		std::cout << "string is not valid\n";
	}
	std::cout << file << std::endl;	
	
	int width = script->get_number(L, "player.width");
	if(width == -1)
	{
		std::cout << "not a valid number\n";
	}
	std::cout << width << std::endl;
	
	int dead = script->get_boolean(L, "player.dead");
	if(dead == -1)
	{
		std::cout << "not a valid boolean\n";
	}
	std::cout << dead << "\n";	
	
    lua_CFunction f = script->get_function(L, "dokun.close");
	if(f == nullptr)
	{
		std::cout << "not a valid function\n";
	}
	f(L);	
*/
#endif
