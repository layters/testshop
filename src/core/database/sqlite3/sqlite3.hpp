#pragma once

#ifndef SQLITE3_HPP_NEROSHOP
#define SQLITE3_HPP_NEROSHOP

constexpr const char* SQLITE3_TAG = "\033[1;36m[sqlite3]:\033[0m ";

#include <sqlite3.h>
#include <nlohmann/json.hpp>

#include <iostream>
#include <string>
#include <vector> // std::vector

namespace neroshop {

namespace db {

class Sqlite3 {
public:
    Sqlite3();
	Sqlite3(const std::string& filename);
	~Sqlite3();
	bool open(const std::string& filename);
	void close();
	int execute(const std::string& command);
	int execute_params(const std::string& command, const std::vector<std::string>& args);
	// getters
	static std::string get_sqlite_version();
    sqlite3 * get_handle() const;
    std::string get_file() const;
	void * get_blob(const std::string& command);
	void * get_blob_params(const std::string& command, const std::vector<std::string>& args);    
	std::string get_text(const std::string& command);// const;
	std::string get_text_params(const std::string& command, const std::vector<std::string>& args);// const;
	int get_integer(const std::string& command);
	int get_integer_params(const std::string& command, const std::vector<std::string>& args);
	double get_real(const std::string& command); // NOTE: both floats and doubles are of the 'real' datatype
	double get_real_params(const std::string& command, const std::vector<std::string>& args);
	std::vector<std::string> get_rows(const std::string& command);
    // boolean
    bool is_open() const;
    bool table_exists(const std::string& table_name);
    //bool rowid_exists(const std::string& table_name, int rowid);
    std::pair<int, std::string> get_error() const; // returns the error result of the last query
    static std::string get_select(); // returns the result of the last select statement 
private:
	sqlite3 * handle;
	bool opened;
	std::string filename;
	static int callback(void *not_used, int argc, char **argv, char **az_col_name);
	std::vector<std::pair<int, std::string>> logger; // arg 1 = error code, arg 2 = error message 
    static nlohmann::json json_object;
};

}

}
#endif
