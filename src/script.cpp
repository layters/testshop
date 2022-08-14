#include "script.hpp"

neroshop::Script::Script(void)
{
	//Factory::get_script_factory()->store(this);
}
//////////
neroshop::Script::Script(lua_State *L, const std::string& file_name) : Script()
{
	if(load(L, file_name) == 0) {
		neroshop::print("Could not load from file: " + file_name);
	}
}
//////////
neroshop::Script::~Script(void)
{
	//Factory::get_script_factory()->release(this);
}
//////////
std::vector<std::string> neroshop::Script::cache	({});
//////////
bool neroshop::Script::load(lua_State * L, const std::string& file_name)
{
	if(!std::filesystem::is_regular_file(file_name))
	{
		return false;
	}
	if(is_script(file_name)) // checks if script object is already attached to a file (one file per script_ptr)
	{
	    //neroshop::print(file_name + " has already been loaded");
		return true;
	}
    if(luaL_dofile(L, file_name.c_str()) != 0)
    {
		lua_error(L);
        return false;
    }
	cache.push_back(file_name); // stores file only once to cache
	this->file = file_name; // save file if load succeeds
	return true;
}
//////////
int neroshop::Script::load(lua_State *L) // non static
{ 
	luaL_checktype(L, 1, LUA_TTABLE);
	luaL_checktype(L, 2, LUA_TSTRING);
	lua_getfield(L, 1, "udata");
	if(lua_isuserdata(L, -1))
	{
		Script * script = *static_cast<Script **>(lua_touserdata(L, -1));	
	    lua_pushboolean(L, script->load (L, lua_tostring(L, 2)));
	    return 1;
	}
	lua_pushboolean(L, false);
	return 1;
}
//////////
bool neroshop::Script::load_script(lua_State *L, const std::string& file_name)
{	
	Script script_ptr;
	return script_ptr.load (L, file_name);
}
//////////
int neroshop::Script::load_script(lua_State *L) // static
{
	lua_pushboolean(L, neroshop::Script::load_script (L, lua_tostring(L, -1)));
	return 1;
}
//////////
bool neroshop::Script::load_directory(lua_State * L, const std::string& path)
{
	if(!std::filesystem::is_directory(path)) {
	    std::filesystem::create_directories(path);
	}
	std::vector<std::string> scripts = neroshop::filesystem::get_dir(path, "*lua");
	for(int i = 0; i < scripts.size(); i++)
	{
		Script script_ptr;
		scripts[i] = path + "/" + scripts[i];                                                              //std::clog << "opening file " << scripts[i] << std::endl;
        if(!script_ptr.load(L, scripts[i]))
		{
			return false;
		}			
	}
	return true;	
}
//////////
int neroshop::Script::load_directory(lua_State *L)
{
	lua_pushboolean( L, neroshop::Script::load_directory( L, lua_tostring(L, -1) ) );
	return 1;
}
//////////
bool neroshop::Script::execute(lua_State * L, const std::string& command)
{
	if(luaL_dostring(L, command.c_str()) != 0) {
		return false;
	}
    return true;	
}
//////////
int neroshop::Script::execute(lua_State *L)
{
	lua_pushboolean(L, neroshop::Script::execute(L, lua_tostring(L, -1)));
	return 1;	
}
//////////
//////////
//////////
//////////
//////////
void neroshop::Script::unload(std::string file_name)
{
	for(int i = 0; i < cache.size(); i++)
	{
        if(cache[i] == file_name) 
		{	
			// remove from storage
			cache.erase(cache.begin() + i);
		}		
	}
}
//////////
void neroshop::Script::generate(const char * script) // generate a main.lua (sets up a window, draws all created objects, etc.)
{
	if( !script)
	{
	    script = "main.lua";
	}
    std::ofstream file(script, std::ios::out | std::ios::binary);
    if (!file.is_open()) 
	{
		std::cerr << "(ERROR): Could not generate script" << std::endl;
		return;
	}
	//////////
	file << "require('dokun')" << std::endl;
	file << "dokun:start()" << std::endl;
	file << "" << std::endl;
	file << "WINDOW_MODE = 0" << std::endl;
	file << "FULL_SCREEN = 1" << std::endl;
	file << "" << std::endl;
	file << "-- Create a window" << std::endl;
	file << "window = Window:new()" << std::endl;
	file << "window:create('Window', 1280, 720, WINDOW_MODE)" << std::endl;
	file << "window:show()" << std::endl;
	file << "" << std::endl;
	file << "while window:is_open() do" << std::endl;
	file << "   window:resize(window:get_client_width(), window:get_client_height())" << std::endl;
	file << "   window:clear()" << std::endl;
	file << "" << std::endl;
	file << "" << std::endl;
	file << "   window:update()" << std::endl;
	file << "end" << std::endl;
	file << "window:destroy()" << std::endl;
	file << "dokun:close  ()" << std::endl;	
	//////////
	file.close();
}                                  
//////////
int neroshop::Script::generate(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
	luaL_optstring(L, 2, "main.lua");
	lua_getfield(L, 1, "udata");
	if(lua_isuserdata(L, -1))
	{
		Script * script = *static_cast<Script **>(lua_touserdata(L, -1));
		script->generate(lua_tostring(L, 2));
	}	
	return 0;
}
//////////
void neroshop::Script::write(std::string code)
{
	if(!is_script()) // not loaded?
	{
		neroshop::print("Could not write to script");
		return;
	}
	std::ofstream file(get_file(), std::ios::app);
	if(!file.is_open())
	{
		neroshop::print("Could not write to " + get_file());
		return;
	}
	file << " " << code << std::endl;
	file.close();
}  
//////////
int neroshop::Script::write(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
	luaL_checktype(L, 2, LUA_TSTRING);
	lua_getfield(L, 1, "udata");
	if(lua_isuserdata(L, -1))
	{
		Script * script = *static_cast<Script **>(lua_touserdata(L, -1));
		script->write(lua_tostring(L, 2));
	}	
	return 0;
}
//////////
void neroshop::Script::copy(std::string file_name)
{
	std::ifstream file(file_name.c_str());
	if(!file.is_open())
	{
	    return;
	}
	std::stringstream stream;
	stream << file.rdbuf(); // dump file contents	
	
	content = stream.str();
} // get_content() -> neroshop::string::replace()   you can replace any string from the lua file with anything
//////////
int neroshop::Script::copy(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
	luaL_checktype(L, 2, LUA_TSTRING);
    lua_getfield(L, 1, "udata");
	if(lua_isuserdata(L, -1))
	{
		Script * script = *static_cast<Script **>(lua_touserdata(L, -1));
		script->copy(lua_tostring(L, 2));
	}		
	return 0;
}
//////////
//////////
bool neroshop::Script::save(lua_State *L, const std::string& table) // saves a table and its keys to a text file
{
    if(table.empty()) return false;
	std::string file_name = table + ".txt";
	std::ofstream file(file_name.c_str());
	if(!file.is_open())
		return false;

	lua_getglobal(L, table.c_str()); // get table    1
    if(lua_type(L, -1) != LUA_TTABLE) return false; // not a valid table
	lua_pushnil(L); // get first key
    while(lua_next(L, 2) != 0) // key at -2, value at -1
    {							
	    std::string key = lua_tostring(L, -2); // get key as string
	    if(lua_type(L, -1) == LUA_TTHREAD)
		{
			lua_getglobal(L, "tostring");
			lua_pushvalue(L, -2);
			lua_call(L, 1, 1);
				
			std::string value = lua_tostring(L, -1);
			std::string thread = key + " =    " + value + "    -- " + "thread\n";
			file.write(thread.c_str(), thread.size());			
			
			lua_pop(L, 1); // pop string					
		}
		if(lua_type(L, -1) == LUA_TFUNCTION)
		{
			lua_getglobal(L, "tostring");
		    lua_pushvalue(L, -2);
			lua_call(L, 1, 1);
				
			std::string value = lua_tostring(L, -1);
			std::string function = key + " =    " + value + "    -- " + "function\n";
			file.write(function.c_str(), function.size());			
			
			lua_pop(L, 1); // pop string					
		}
		if(lua_type(L, -1) == LUA_TUSERDATA)
		{
			lua_getglobal(L, "tostring");
		    lua_pushvalue(L, -2);
			lua_call(L, 1, 1);
					
			std::string value = lua_tostring(L, -1);
			std::string udata = key + " =    " + value + "    -- " + "userdata\n";
			file.write(udata.c_str(), udata.size());
			
			lua_pop(L, 1); // pop string					
		}
		if(lua_type(L, -1) == LUA_TLIGHTUSERDATA)
		{
			lua_getglobal(L, "tostring");
			lua_pushvalue(L, -2);
			lua_call(L, 1, 1);
				
			std::string value = lua_tostring(L, -1);
			std::string lightuserdata = key + " =    " + value + "    -- " + "lightuserdata\n";
			file.write(lightuserdata.c_str(), lightuserdata.size());			
			
			lua_pop(L, 1); // pop string					
		}
		if(lua_type(L, -1) == LUA_TTABLE)
		{
			lua_getglobal(L, "tostring");
		    lua_pushvalue(L, -2);
			lua_call(L, 1, 1);
			
			std::string value = lua_tostring(L, -1);
			std::string table_info = key + " =    " + value + "    -- " + "table\n";
			file.write(table_info.c_str(), table_info.size());			
		
			lua_pop(L, 1); // pop string	
		}			
        if(lua_type(L, -1) == LUA_TSTRING)
		{
			
			std::string value = lua_tostring(L, -1);
			std::string str_info = key + " =    \"" + value + "\"    -- " + "string\n";
			file.write(str_info.c_str(), str_info.size());	    
		}
	    if(lua_type(L, -1) == LUA_TBOOLEAN)
		{
            std::string value;
			if(lua_toboolean(L, -1) != 0)
			{
		 	    value = "true";
			}  
			if(lua_toboolean(L, -1) == 0)
			{
			    value = "false";
			}
			std::string bool_info = key + " =    " + value + "    -- " + "boolean\n";
			file.write(bool_info.c_str(), bool_info.size());
		}
		if(lua_type(L, -1) == LUA_TNUMBER)
		{
			std::string value = lua_tostring(L, -1);
			std::string number_info = key + " =    " + value + "    -- " + "number\n";
			file.write(number_info.c_str(), number_info.size());			
		}
        lua_pop(L, 1); // pop key
	}
	// close file
	file.close(); 
	return true;
}
//////////
int neroshop::Script::save(lua_State *L)
{
	lua_pushboolean(L, save(L, lua_tostring(L, -1)) );
	return 1;
}
//////////
void neroshop::Script::save_cache()
{
    if(cache.empty()) return; // cache is empty, so return (since there is nothing to save)
    ////////////////////////
    // open file for reading
    std::ifstream rfile;
    rfile.open ("script_cache.txt", std::ios::in | std::ios::app);
    // if file already exists
    std::stringstream stream;
    std::vector<std::string> content_in_file;
    if(rfile.is_open()) {
	    stream << rfile.rdbuf(); // dump file contents	
	    content_in_file = neroshop::string::split(stream.str(), "\n"); // split each line
	    // cache can ONLY hold 5 filenames
	    if(content_in_file.size() >=5) content_in_file.clear(); // erase all file contents if the number of filenames reach 5
	    // look for duplicate names and remove them ...
	    for(int i = 0; i < content_in_file.size(); i++)
	    {
	        for(int j = 0; j < cache.size(); j++) 
	        {
	            if(cache[j] == content_in_file[i]) { cache[j].clear(); } // remove duplicate filenames previously saved to cache file: "script_cache.txt" //if(neroshop::string::contains(content_in_file[i], ".lua")) std::cout << "Found recent files: " << content_in_file[i] << std::endl; // if its a lua file, print filename
	        }
	    }
    }
    if(rfile.is_open()) rfile.close();
    ////////////////////////
    // open file for writing
    std::ofstream cfile;
    cfile.open ("script_cache.txt", std::ios::out | std::ios::app); // std::ios::out is default mode for ofstream (writing to file)
    if(!cfile.is_open()) {neroshop::print(std::string("neroshop::Script::save_cache : Could not open file: ") + "script_cache.txt");return;}
    for(int i = 0; i < cache.size(); i++)
    {
        if(!cache[i].empty()) cfile << cache[i] << std::endl;
    }
    cfile.close();    
}
//////////
// getters
//////////
//////////
void neroshop::Script::get_table(lua_State *L, const std::string& table) // can also get subtables
{
	if(!neroshop::string::contains(table, ".")) // no dots
	{
	    lua_getglobal(L, table.c_str());
		if(lua_type(L, -1) != LUA_TTABLE) // not a table?   YOU can only get tables with this function
		{
			lua_pop(L, 1); // pop it from stack
			return;
		}
	}
	// contains dots
    std::vector<std::string> list = neroshop::string::split(table, ".");
	if(!list.empty())
	{
		lua_getglobal(L, list[0].c_str()); // get first table
		for(unsigned int i = 1; i <= (list.size() - 1); i++)
		{
			lua_getfield(L, -1, list[i].c_str()); // get sub_table(s)		
		}
		if(lua_type(L, -1) != LUA_TTABLE) // not a table?   YOU can only get tables with this function
		{
			lua_pop(L, 1); // pop it from stack
			return;
		}		
	}		
}
//////////
//////////
std::string neroshop::Script::get_string(lua_State * L, const std::string& key)
{
    if(!neroshop::string::contains(key, "."))
	{
		lua_getglobal(L, key.c_str());
		if(!lua_tostring(L, -1))
		{
			return "";
		}
		return lua_tostring(L, -1);
	}
	std::vector<std::string> list = neroshop::string::split(key, ".");
	if(!list.empty())
	{
		lua_getglobal(L, list[0].c_str());
		for(unsigned int i = 1; i <= (list.size() - 1); i++)
		{
			lua_getfield(L, -1, list[i].c_str());		
			if(lua_type(L, -1) == LUA_TSTRING) // .field is a string
			{
				if(!lua_tostring(L, -1)) // string is null
				{
					return "";
				}
				return lua_tostring(L, -1);
			}			
		}
	}
	return "";	
}
//////////
int neroshop::Script::get_boolean(lua_State * L, const std::string& key)
{
    if(!neroshop::string::contains(key, "."))
	{
		lua_getglobal(L, key.c_str());
		if(!lua_isboolean(L, -1)) // not a boolean
		{
			return -1;
		}
		return lua_toboolean(L, -1);
	}
	std::vector<std::string> list = neroshop::string::split(key, ".");
	if(!list.empty())
	{
		lua_getglobal(L, list[0].c_str());
		for(unsigned int i = 1; i <= (list.size() - 1); i++)
		{
			lua_getfield(L, -1, list[i].c_str());
			if(lua_type(L, -1) == LUA_TBOOLEAN)
			{
				return lua_toboolean(L, -1);
			}				
		}
	}
	return -1;	
}
//////////
double neroshop::Script::get_number(lua_State * L, const std::string& key)
{
    if(!neroshop::string::contains(key, "."))
	{
		lua_getglobal(L, key.c_str());
		if(!lua_isnumber(L, -1)) // not a boolean
		{
			return -1;
		}
		return lua_tonumber(L, -1);
	}
	std::vector<std::string> list = neroshop::string::split(key, ".");
	if(!list.empty())
	{
		lua_getglobal(L, list[0].c_str());
		for(unsigned int i = 1; i <= (list.size() - 1); i++)
		{
			lua_getfield(L, -1, list[i].c_str());
			if(lua_type(L, -1) == LUA_TNUMBER)
			{
				return lua_tonumber(L, -1);
			}				
		}
	}
	return -1;		
}
//////////
void * neroshop::Script::get_userdata(lua_State * L, const std::string& key)
{
	if(!neroshop::string::contains(key, "."))
	{
		lua_getglobal(L, key.c_str());
		if(!lua_isuserdata(L, -1)) // either full or light userdata
		{
			return nullptr;
		}
		return lua_touserdata(L, -1);
	}
	std::vector<std::string> list = neroshop::string::split(key, ".");
	if(!list.empty())
	{
		lua_getglobal(L, list[0].c_str());
		for(unsigned int i = 1; i <= (list.size() - 1); i++)
		{
			lua_getfield(L, -1, list[i].c_str());
            if(lua_type(L, -1) == LUA_TUSERDATA)
			{
				return lua_touserdata(L, -1);
			}						
		}
	}
	return nullptr;	
}
//////////
lua_CFunction neroshop::Script::get_function(lua_State * L, const std::string& key)
{
	if(!neroshop::string::contains(key, "."))
	{
		lua_getglobal(L, key.c_str());
		if(!lua_iscfunction(L, -1)) // not a C/C++ function
		{
			return nullptr;
		}
		return lua_tocfunction(L, -1);
	}	
    std::vector<std::string> list = neroshop::string::split(key, ".");
	if(!list.empty())
	{
		lua_getglobal(L, list[0].c_str());
		for(unsigned int i = 1; i <= (list.size() - 1); i++)
		{
			lua_getfield(L, -1, list[i].c_str());
            if(lua_iscfunction(L, -1))
			{
				return lua_tocfunction(L, -1);
			}						
		}
	}	
	return nullptr;
}
//////////
lua_State * neroshop::Script::get_thread (lua_State * L, const std::string& key)
{
    if(!neroshop::string::contains(key, "."))
	{
		lua_getglobal(L, key.c_str());
		if(!lua_isthread(L, -1))
		{
			return nullptr;
		}
		return lua_tothread(L, -1);
	}
    std::vector<std::string> list = neroshop::string::split(key, ".");
	if(!list.empty())
	{
		lua_getglobal(L, list[0].c_str());
		for(unsigned int i = 1; i <= (list.size() - 1); i++)
		{
			lua_getfield(L, -1, list[i].c_str());
			if(lua_type(L, -1) == LUA_TTHREAD)
			{
				return lua_tothread(L, -1);
			}
		}
	}		
	return nullptr; // default
}
//////////
void * neroshop::Script::get_pointer(lua_State * L, const std::string& key)
{
    if(!neroshop::string::contains(key, "."))
	{
		lua_getglobal(L, key.c_str());
        if(lua_isuserdata(L, -1) || lua_isthread(L, -1) || lua_isfunction(L, -1)) // must be either a userdata, thread, or a function
		{
		    return const_cast<void *>(lua_topointer(L, -1));
		}
	}
    std::vector<std::string> list = neroshop::string::split(key, ".");
	if(!list.empty())
	{
		lua_getglobal(L, list[0].c_str());
		for(unsigned int i = 1; i <= (list.size() - 1); i++)
		{
			lua_getfield(L, -1, list[i].c_str());
			if(lua_isuserdata(L, -1) || lua_isthread(L, -1) || lua_isfunction(L, -1))
			{
				return const_cast<void *>(lua_topointer(L, -1));
			}
		}
	}		
	return nullptr; // default
}
//////////
std::vector<std::string> neroshop::Script::get_table_string(lua_State * L, const std::string& table)
{
	std::vector<std::string> string_array;
    neroshop::Script::get_table(L, table.c_str()); // get table
	if(lua_istable(L, 1))
	{
	// get table size
    #ifdef DOKUN_LUA51
	    int size = lua_objlen(L, -1);
	#endif
	#ifndef DOKUN_LUA51
		int size = luaL_len(L, -1);
	#endif		
		for(int i = 1; i <= size; i++)
		{
			lua_rawgeti(L, -1, i); // push string from table
			string_array.push_back(lua_tostring(L, -1));
			lua_pop(L, 1); // pop string
		}			
	}
    return 	string_array;
}
//////////
std::vector<double> neroshop::Script::get_table_number(lua_State * L, const std::string& table)
{
	std::vector<double> number_array;
    neroshop::Script::get_table(L, table.c_str()); // get table
	if(lua_istable(L, 1))
	{
	// get table size
    #ifdef DOKUN_LUA51
	    int size = lua_objlen(L, -1);
	#endif
	#ifndef DOKUN_LUA51
		int size = luaL_len(L, -1);
	#endif		
		for(int i = 1; i <= size; i++)
		{
			lua_rawgeti(L, -1, i); // push number from table
			number_array.push_back(lua_tonumber(L, -1));
			lua_pop(L, 1); // pop number
		}			
	}
	return number_array;
}
//////////
std::vector<int> neroshop::Script::get_table_integer(lua_State * L, const std::string& table)
{
	std::vector<int> number_array;
    neroshop::Script::get_table(L, table.c_str()); // get table
	if(lua_istable(L, 1))
	{
	// get table size
    #ifdef DOKUN_LUA51
	    int size = lua_objlen(L, -1);
	#endif
	#ifndef DOKUN_LUA51
		int size = luaL_len(L, -1);
	#endif		
		for(int i = 1; i <= size; i++)
		{
			lua_rawgeti(L, -1, i); // push number from table
			number_array.push_back(lua_tointeger(L, -1));
			lua_pop(L, 1); // pop number
		}			
	}
	return number_array;
}
//////////
std::vector<float> neroshop::Script::get_table_float(lua_State * L, const std::string& table)
{
	std::vector<float> number_array;
    neroshop::Script::get_table(L, table.c_str()); // get table
	if(lua_istable(L, 1))
	{
	// get table size
    #ifdef DOKUN_LUA51
	    int size = lua_objlen(L, -1);
	#endif
	#ifndef DOKUN_LUA51
		int size = luaL_len(L, -1);
	#endif		
		for(int i = 1; i <= size; i++)
		{
			lua_rawgeti(L, -1, i); // push number from table
			number_array.push_back(lua_tonumber(L, -1));
			lua_pop(L, 1); // pop number
		}			
	}
	return number_array;
}
//////////
std::vector<double> neroshop::Script::get_table_double(lua_State * L, const std::string& table)
{
    return get_table_number(L, table);
}
//////////
std::vector<const void *> neroshop::Script::get_table_pointer(lua_State * L, const std::string& table) // can be a userdata, a table, a thread, or a function
{
	std::vector<const void *> pointer_array;
    neroshop::Script::get_table(L, table.c_str()); // get table
	if(lua_istable(L, 1))
	{
	// get table size
    #ifdef DOKUN_LUA51
	    int size = lua_objlen(L, -1);
	#endif
	#ifndef DOKUN_LUA51
		int size = luaL_len(L, -1);
	#endif		
		for(int i = 1; i <= size; i++)
		{
			lua_rawgeti(L, -1, i); // push  from table
			pointer_array.push_back(lua_topointer(L, -1)); // can be a userdata, a table, a thread, or a function
			lua_pop(L, 1); // pop 
		}			
	}
    return 	pointer_array;
}
//////////
bool neroshop::Script::call(lua_State * L, const std::string& function, int returns)
{
	if(!neroshop::string::contains(function, "."))
	{
		lua_getglobal(L, function.c_str());
		if(!lua_isfunction(L, -1)) // not a function
		{
			return false;
		}
		lua_call(L, 0, returns);
		return true;
	}
    std::vector<std::string> list = neroshop::string::split(function, ".");
	if(!list.empty())
	{
		lua_getglobal(L, list[0].c_str());
		for(unsigned int i = 1; i <= (list.size() - 1); i++)
		{
			lua_getfield(L, -1, list[i].c_str());
            if(lua_isfunction(L, -1))
			{
				lua_call(L, 0, returns); // call function (function must have 0 arguments)
				return true;
			}						
		}
	}	
    return false;	
}
//////////
std::string neroshop::Script::get_type(lua_State * L, const std::string& object) // can only handle up to 10 nested tables MAX
{
    lua_settop (L, 0);// clear stack
    if(!neroshop::string::contains(object, "."))
	{
		lua_getglobal(L, object.c_str());
		if(lua_type(L, -1) == LUA_TTHREAD)
		{
			return "thread";
		}
		if(lua_isfunction(L, -1))
		{
			return "function";
		}			
		if(lua_type(L, -1) == LUA_TUSERDATA)
		{
			return "userdata";
		}
		if(lua_type(L, -1) == LUA_TLIGHTUSERDATA)
		{
			return "lightuserdata";
		}
		if(lua_type(L, -1) == LUA_TTABLE)
		{
			return "table";
		}
		if(lua_type(L, -1) == LUA_TSTRING)
		{
			return "string";
		}
		if(lua_type(L, -1) == LUA_TBOOLEAN)
		{
			return "boolean";
		}
		if(lua_type(L, -1) == LUA_TNUMBER)
		{
			return "number";
		}
		if(lua_type(L, -1) == LUA_TNIL)
		{
			return "nil";
		}
		if(lua_type(L, -1) == LUA_TNONE)
		{
			return "none";
		}
	}
    std::vector<std::string> list = neroshop::string::split(object, ".");
	if(!list.empty())
	{
	    lua_getglobal(L, list[0].c_str()); // get first table
		for(unsigned int i = 1; i <= (list.size() - 1); i++)
		{
			lua_getfield(L, -1, list[i].c_str()); // second table inside first table
			//////////////////////////////////
			if(lua_type(L, -1) == LUA_TTABLE) {
			    if(list.size() == 3) {
			        // 0 - table
                    // 1 - table
                    // 2 - value we are trying to get
			        lua_getfield(L, -1, list[2].c_str());
			    } // get third field (from second field)
			    if(list.size() == 4) {
			        // 0 - table
                    // 1 - table
                    // 2 - table
                    // 3 - value we are trying to get
			        lua_getfield(L, -1, list[2].c_str());
			        lua_getfield(L, -1, list[3].c_str());
			    }
			    if(list.size() == 5) {
			        // 0 - table
                    // 1 - table
                    // 2 - table
                    // 3 - table
                    // 4 - value we are trying to get
			        lua_getfield(L, -1, list[2].c_str()); 
			        lua_getfield(L, -1, list[3].c_str());
			        lua_getfield(L, -1, list[4].c_str());
			    }
			    if(list.size() == 6) {
			        // 0 - table
                    // 1 - table
                    // 2 - table
                    // 3 - table
                    // 4 - table
                    // 5 - value we are trying to get
			        lua_getfield(L, -1, list[2].c_str()); 
			        lua_getfield(L, -1, list[3].c_str());
			        lua_getfield(L, -1, list[4].c_str());
			        lua_getfield(L, -1, list[5].c_str());
			    }
			    if(list.size() == 7) {
			        // 0 - table
                    // 1 - table
                    // 2 - table
                    // 3 - table
                    // 4 - table
                    // 5 - table
                    // 6 - value we are trying to get
			        lua_getfield(L, -1, list[2].c_str()); 
			        lua_getfield(L, -1, list[3].c_str());
			        lua_getfield(L, -1, list[4].c_str());
			        lua_getfield(L, -1, list[5].c_str());
			        lua_getfield(L, -1, list[6].c_str());
			    }
			    if(list.size() == 8) {
			        // 0 - table
                    // 1 - table
                    // 2 - table
                    // 3 - table
                    // 4 - table
                    // 5 - table
                    // 6 - table
                    // 7 - value we are trying to get
			        lua_getfield(L, -1, list[2].c_str()); 
			        lua_getfield(L, -1, list[3].c_str());
			        lua_getfield(L, -1, list[4].c_str());
			        lua_getfield(L, -1, list[5].c_str());
			        lua_getfield(L, -1, list[6].c_str());
			        lua_getfield(L, -1, list[7].c_str());
			    }
			    if(list.size() == 9) {
			        // 0 - table
                    // 1 - table
                    // 2 - table
                    // 3 - table
                    // 4 - table
                    // 5 - table
                    // 6 - table
                    // 7 - table
                    // 8 - value we are trying to get
			        lua_getfield(L, -1, list[2].c_str()); 
			        lua_getfield(L, -1, list[3].c_str());
			        lua_getfield(L, -1, list[4].c_str());
			        lua_getfield(L, -1, list[5].c_str());
			        lua_getfield(L, -1, list[6].c_str());
			        lua_getfield(L, -1, list[7].c_str());
			        lua_getfield(L, -1, list[8].c_str());
			    }
			    if(list.size() == 10) {
			        // 0 - table
                    // 1 - table
                    // 2 - table
                    // 3 - table
                    // 4 - table
                    // 5 - table
                    // 6 - table
                    // 7 - table
                    // 8 - table
                    // 9 - value we are trying to get
			        lua_getfield(L, -1, list[2].c_str()); 
			        lua_getfield(L, -1, list[3].c_str());
			        lua_getfield(L, -1, list[4].c_str());
			        lua_getfield(L, -1, list[5].c_str());
			        lua_getfield(L, -1, list[6].c_str());
			        lua_getfield(L, -1, list[7].c_str());
			        lua_getfield(L, -1, list[8].c_str());			    
                    lua_getfield(L, -1, list[9].c_str());
			    }
			}
			//////////////////////////////////			
			if(lua_type(L, -1) == LUA_TTHREAD)
			{
				return "thread";
			}
			if(lua_isfunction(L, -1))
			{
				return "function";
			}			
			if(lua_type(L, -1) == LUA_TUSERDATA)
			{
				return "userdata";
			}
			if(lua_type(L, -1) == LUA_TLIGHTUSERDATA)
			{
				return "lightuserdata";
			}
			if(lua_type(L, -1) == LUA_TTABLE)
			{
				return "table";
			}
			if(lua_type(L, -1) == LUA_TSTRING)
			{
				return "string";
			}
			if(lua_type(L, -1) == LUA_TBOOLEAN)
			{
				return "boolean";
			}
			if(lua_type(L, -1) == LUA_TNUMBER)
			{
				return "number";
			}
			if(lua_type(L, -1) == LUA_TNIL)
			{
				return "nil";
			}
			if(lua_type(L, -1) == LUA_TNONE)
			{
				return "none";
			}
		}
	}		
	return "nil";
}
//////////
//////////
//////////
int neroshop::Script::table_not_mt(lua_State *L, std::string class_name) // soon to be replaced by neroshop::Script::table
{
	lua_newtable(L);
	lua_newtable(L); 
	lua_pushvalue(L, -2);
	lua_setfield(L, -2, "__index"); 
	std::string mt(class_name + "_mt");
	lua_setglobal(L, mt.c_str()); 
    lua_setglobal(L, class_name.c_str());	
	return 0;
}
////////////
int neroshop::Script::table(lua_State *L, std::string class_name) // the base table itself will also be a metatable
{
	lua_newtable(L); // create {} table: 0x242e040
	lua_pushvalue(L, -1); // push the same table again {} table: 0x242e040
	lua_setfield(L, -2, "__index"); // {__index = {}} 
	lua_setglobal(L, class_name.c_str()); // Monster = {__index= {}}
	return 0;
}
////////////
int neroshop::Script::function(lua_State *L, std::string table_name, std::string function_name, lua_CFunction function)
{
	lua_getglobal(L, table_name.c_str());
	if(!lua_istable(L, -1))	
	{
		lua_pop(L, 1);
		luaL_error(L, "Table %s not found", table_name.c_str());
	}
	lua_pushcfunction(L, function); 
	lua_setfield(L, -2, function_name.c_str());
    lua_pop(L, 1);
	return 0;
}
////////////
int neroshop::Script:: member (lua_State *L, std::string table_name, std::string value_name, std::string value, int type)
{
	if( type == LUA_TSTRING )
	{
		lua_getglobal(L, table_name.c_str()); 
	    if(!lua_istable(L, -1))	
	    {
			lua_pop(L, 1);
		    luaL_error(L, "Table %s not found", table_name.c_str());
	    }
		lua_pushstring(L, value.c_str());
		lua_setfield(L, -2, value_name.c_str());
        lua_pop(L, 1);
	}
	if( type == LUA_TTABLE )
	{
		lua_getglobal(L, table_name.c_str());
	    if(!lua_istable(L, -1))	
	    {
		    lua_pop(L, 1);
			luaL_error(L, "Table %s not found", table_name.c_str());
	    }
		lua_getglobal(L, value.c_str());
		if(lua_istable(L, -1))
		{
			lua_setfield(L, -2, value_name.c_str());
			lua_pop(L, 1);
		}
		else
		{ 
		    lua_pop(L, 1); 
			lua_createtable(L, 0, 0);
			lua_setfield(L, -3, value_name.c_str());
			lua_pop(L, 1);
		}
	} 	
	lua_settop(L, 0);
	return 0;
}
////////////
int neroshop::Script:: member (lua_State *L, std::string table_name, std::string value_name, int value, int type)
{
	if( type == LUA_TNUMBER )
	{
		lua_getglobal(L, table_name.c_str());
	    if(!lua_istable(L, -1))	
	    {
		    
		    lua_pop(L, 1);
			luaL_error(L, "Table %s not found", table_name.c_str());
	    }
		lua_pushnumber(L, value);
		lua_setfield(L, -2, value_name.c_str());
        lua_pop(L, 1);
	}
	if( type == LUA_TBOOLEAN )
	{
		lua_getglobal(L, table_name.c_str());
	    if(!lua_istable(L, -1))	
	    {
		    
		    lua_pop(L, 1);
			luaL_error(L, "Table %s not found", table_name.c_str());
	    }
		lua_pushboolean(L, value);
		lua_setfield(L, -2, value_name.c_str());
        lua_pop(L, 1);
	}
	lua_settop(L, 0);
	return 0;
}
////////////
int neroshop::Script:: global (lua_State *L, std::string function_name, lua_CFunction function)
{
	lua_pushcfunction(L, function);
	lua_setglobal(L, function_name.c_str());
	// lua_register(L, function_name.c_str(), function );
	return 0;
}
////////////
int neroshop::Script:: global (lua_State *L, std::string variable_name, std::string value)
{
	lua_pushstring(L, value.c_str());
	lua_setglobal(L, variable_name.c_str());
	return 0;
}
////////////
int neroshop::Script:: global (lua_State *L, std::string variable_name, int value)
{
	lua_pushnumber(L, value);
	lua_setglobal(L, variable_name.c_str());
	return 0;
}
////////////
int neroshop::Script:: global (lua_State *L, std::string variable_name, bool value)
{
	lua_pushboolean(L, value);
	lua_setglobal(L, variable_name.c_str());
	return 0;
}
////////////
int neroshop::Script:: inherit (lua_State *L, std::string base_table, std::string sub_table)
{
	lua_getglobal(L, sub_table.c_str());
	if(!lua_istable(L, -1))	
	{
		lua_pop(L, 1);
	    luaL_error(L, "Table %s not found", sub_table.c_str()); 
	}
	lua_getglobal(L, base_table/*.append("_mt")*/.c_str()); // _mt soon to be removed
	if(!lua_istable(L, -1))	
	{
		lua_pop(L, 2);
		luaL_error(L, "Table %s not found", base_table.c_str());
	}
	lua_setmetatable(L, -2);
	lua_pop(L, 1);
	return 0;
}
////////////
int neroshop::Script::attach(lua_State *L, std::string base_table, std::string sub_table)
{
    lua_getglobal(L, base_table.c_str());
	if(!lua_istable(L, -1))	
	{
		lua_pop(L, 1);
		luaL_error(L, "Table %s not found", base_table.c_str());
	}
	lua_getglobal(L, sub_table.c_str());
	if(lua_istable(L, -1))
	{
		lua_setfield(L, -2, sub_table.c_str());
		lua_pop(L, 1);
	}
	else
	{       
		    lua_pop(L, 1);
			lua_createtable(L, 0,0);
			lua_setfield(L, -2, sub_table.c_str());
			lua_newtable(L);
			lua_getfield(L, -2, sub_table.c_str());
			lua_setfield(L, -2, "__index");
			lua_setglobal(L, sub_table/*.append("_mt")*/.c_str()); // _mt soon to be removed
			lua_pop(L, 1);
	}	
	return 0;
}
////////////
//////////
//////////
//////////
//////////
std::string neroshop::Script::get_file()
{
	return file;
}       
//////////         
int neroshop::Script::get_file(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_getfield(L, 1, "udata");
	if(lua_isuserdata(L, -1))
	{
		Script * script = *static_cast<Script **>(lua_touserdata(L, -1));
		lua_pushstring(L, script->get_file().c_str());
		return 1;
	}
	lua_pushnil(L);
	return 1;
}
//////////
std::string neroshop::Script::get_content()
{
	if(!is_script()) // not loaded before?
	{
		return ("");
	}
	copy(get_file()); // updates script by copying file contents
	return (content);
}
//////////
//////////
//////////
int neroshop::Script::get_count()
{
	return cache.size();
} 
//////////
int neroshop::Script::get_count(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "udata");
	if(lua_isuserdata(L, -1))
	{
		Script * script = *static_cast<Script **>(lua_touserdata(L, -1));
		lua_pushnumber(L, script->get_count() );
		return 1 ;
	}	
	lua_pushnil(L);
	return 1;
}
//////////
//////////
//////////
bool neroshop::Script::is_script() // returns true if script has a file name( meaning the script is loaded)
{
	if(get_file().empty()) // no file attached to script object
	{
		return false;
	}
	return true;
}
//////////
bool neroshop::Script::is_script(std::string file_name) // returns true if a script has been loaded (previously)
{
	for (unsigned int i = 0; i < cache.size(); i++)
	{	
		if(cache[i] == file_name)
		{
			return true;
		}
	}
	return false;
}
//////////
int neroshop::Script::is_script(lua_State *L)
{
	lua_pushboolean(L, neroshop::Script::is_script(lua_tostring(L, -1)));
	return 1;
}
//////////
//////////
int neroshop::Script::new_(lua_State *L)
{
    std::string file_name = lua_tostring(L, -1);
    // clear stack
	lua_settop(L, 0);
	// create table
	lua_createtable(L, 0, 0);
	// create mt
	lua_getglobal(L, "Script");
	lua_setmetatable(L, 1);
	// create udata
	Script **script = static_cast<Script **>(lua_newuserdata(L, sizeof(Script *)));
	if(!file_name.empty()) *script = new Script(L, file_name);
	else *script = new Script();
	lua_setfield(L, 1, "udata");
	// non-static load function
	lua_pushcfunction(L, neroshop::Script::load);
	lua_setfield(L, 1, "load");
	// return table
	if(lua_istable(L, -1))
		return 1 ;
	lua_pushnil(L);
	return 1;
}
//////////
