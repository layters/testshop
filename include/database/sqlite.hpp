// filename: db_sqlite.hpp
#ifndef DATABASE_SQLITE_HPP_NEROSHOP // recommended to add unique identifier like _NEROSHOP to avoid naming collision with other libraries
#define DATABASE_SQLITE_HPP_NEROSHOP
#define SQLITE3_TAG "\033[1;36m[sqlite3]:\033[0m "
#define SQLITE3_TAG_ERR "\033[1;36m[sqlite3]:\033[0;91m "
#include <iostream>
#include <cstdarg>
#include <fstream>
#include <sstream>
#include <utility> // std::pair
#include <memory> // std::unique_ptr
#include <vector> // std::vector
#include <stdexcept> // std::runtime_error
// neroshop
#include "debug.hpp"
// sqlite3
extern "C" { 
#include <sqlite3.h>
}

namespace neroshop {
namespace DB {
class SQLite3 {
public:
    SQLite3();
	SQLite3(const std::string& filename);
	~SQLite3();
	bool open(const std::string& filename);
	void close();
	void execute(const std::string& command);
	void execute_params(const std::string& command, const std::vector<std::string>& args);
	// getters
	static std::string get_sqlite_version();
    sqlite3 * get_handle() const;
    static SQLite3 * get_singleton();
	void * get_blob(const std::string& command);
	void * get_blob_params(const std::string& command, const std::vector<std::string>& args);    
	std::string get_text(const std::string& command);// const;
	std::string get_text_params(const std::string& command, const std::vector<std::string>& args);// const;
	int get_integer(const std::string& command);
	int get_integer_params(const std::string& command, const std::vector<std::string>& args);
	float get_real(const std::string& command);
	float get_real_params(const std::string& command, const std::vector<std::string>& args);
    // boolean
    bool table_exists(const std::string& table_name);
private:
	sqlite3 * handle;
	static std::unique_ptr<SQLite3> singleton;
	static int callback(void *not_used, int argc, char **argv, char **az_col_name);
};
}
}
/*
	// Would it be more appropriate if the server handles the database queries instead of the client?
	// The client would send the server a request to execute a SQLite query. This sounds good.
	// Since neroshop will only be using one database file, should we create a singleton object?
*/
#endif
