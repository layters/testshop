#ifndef DATABASE_SQLITE_HPP_NEROSHOP
#define DATABASE_SQLITE_HPP_NEROSHOP

#define SQLITE3_TAG "\033[1;36m[sqlite3]:\033[0m "
#define SQLITE3_TAG_ERR "\033[1;36m[sqlite3]:\033[0;91m "

//#define NEROSHOP_DATABASE_PATH ""
#define NEROSHOP_DATABASE_FILE "data.sqlite3"

#include <sqlite3.h>
#include <iostream>
#include <cstdarg>
#include <fstream>
#include <sstream>
#include <utility> // std::pair
#include <memory> // std::unique_ptr
#include <vector> // std::vector
#include <stdexcept> // std::runtime_error

#include "debug.hpp"

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
	// setters
	//void set_singleton(const SQLite3& singleton); // transfer ownership of singleton to the SQLite3 unique_ptr
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
	double get_real(const std::string& command); // NOTE: both floats and doubles are of the 'real' datatype
	double get_real_params(const std::string& command, const std::vector<std::string>& args);
    // boolean
    bool is_open() const;
    bool table_exists(const std::string& table_name);
    //bool rowid_exists(const std::string& table_name, int rowid);
private:
	sqlite3 * handle;
	static std::unique_ptr<SQLite3> singleton;
	bool opened;
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
