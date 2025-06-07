#include "sqlite3.hpp"

#include "../../tools/logger.hpp"

#include <cstdarg>
#include <fstream>
#include <sstream>
#include <stdexcept> // std::runtime_error

#include <fmt/ranges.h> // fmt::join

namespace neroshop {

namespace db {

//-----------------------------------------------------------------------------

Sqlite3::Sqlite3() : handle(nullptr), opened(false), enable_mutex(false) {}

//-----------------------------------------------------------------------------

Sqlite3::Sqlite3(const std::string& filename, int flags) : Sqlite3()
{
	if(!open(filename, flags)) {
		throw std::runtime_error(std::string("sqlite3_open: ") + std::string(sqlite3_errmsg(handle)));
    }
}

//-----------------------------------------------------------------------------

Sqlite3::~Sqlite3() {
    close();
}

//-----------------------------------------------------------------------------

static void log_flags(int flags) {
    std::vector<std::string> flag_names;

    if (flags & SQLITE_OPEN_READONLY) flag_names.push_back("READONLY");
    if (flags & SQLITE_OPEN_READWRITE) flag_names.push_back("READWRITE");
    if (flags & SQLITE_OPEN_CREATE) flag_names.push_back("CREATE");
    if (flags & SQLITE_OPEN_NOMUTEX) flag_names.push_back("NOMUTEX");
    if (flags & SQLITE_OPEN_FULLMUTEX) flag_names.push_back("FULLMUTEX");
    if (flags & SQLITE_OPEN_SHAREDCACHE) flag_names.push_back("SHAREDCACHE");
    if (flags & SQLITE_OPEN_PRIVATECACHE) flag_names.push_back("PRIVATECACHE");
    if (flags & SQLITE_OPEN_URI) flag_names.push_back("URI");

    if (flag_names.empty()) {
        neroshop::log_info("Opening database with no recognized flags (flags=0x{:X})", flags);
    } else {
        neroshop::log_info("Opening database with flags: {}", fmt::join(flag_names, " | "));
    }
}

//-----------------------------------------------------------------------------

// SQLite database should only need to be opened once per application session and closed once when the application is terminated
bool Sqlite3::open(const std::string& filename, int flags)
{
    if(opened) {
        neroshop::log_warn("database is already opened");
        return true;
    }
    
    // SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE is the behavior that is always used for sqlite3_open() and sqlite3_open16(): https://sqlite.org/c3ref/open.html
	log_flags(flags);
	if(sqlite3_open_v2(filename.c_str(), &handle, flags, nullptr) != SQLITE_OK) {
		close();
		return false;
	}

    // Enable Foreign keys
	execute("PRAGMA foreign_keys = ON;"); // requires version 3.6.19 (2009-10-14)	
	// Enable Write-Ahead Log. This will prevent the database from being locked by allowing multiple readers and one concurrent writer
	execute("PRAGMA journal_mode = WAL;"); // requires version 3.7.0 (2010-07-21)	
	// Set busy timeout to 5000 ms (5 seconds) to avoid "database is locked" errors by waiting for locks (default: 0)
    execute("PRAGMA busy_timeout = 5000;");
    // Set synchronous to NORMAL for better performance with WAL
    execute("PRAGMA synchronous = NORMAL;");
    // Test to see if SQLite is thread-safe
    assert(sqlite3_threadsafe() != 0);
    
	opened = true;
	this->filename = filename;
	return true;
}

//-----------------------------------------------------------------------------

void Sqlite3::close() {
    if(!handle) return;
	
	if(sqlite3_close(handle) == SQLITE_OK) {
	    handle = nullptr;
        opened = false;
        neroshop::log_debug("sqlite3_close: {} closed", filename.empty() ? "database" : filename);
    }
}

//-----------------------------------------------------------------------------

int Sqlite3::execute(const std::string& command) {
    if (enable_mutex) std::lock_guard<std::mutex> lock(db_mutex);
    if(!handle) throw std::runtime_error("database is not connected");
	char * error_message = 0;
	int result = sqlite3_exec(handle, command.c_str(), Sqlite3::callback, 0, &error_message);
	if (result != SQLITE_OK) {
		neroshop::log_error("sqlite3_exec: " + std::string(error_message));
		errors.push_back(std::make_pair(result, std::string(error_message)));
		sqlite3_free(error_message);
		return result;
	}
	return result;
}

//-----------------------------------------------------------------------------

int Sqlite3::execute_params(const std::string& command, const std::vector<std::string>& args) {
    if (enable_mutex) std::lock_guard<std::mutex> lock(db_mutex);
    if(!handle) throw std::runtime_error("database is not connected");
    // Prepare statement
    sqlite3_stmt * statement = nullptr;
    int result = sqlite3_prepare_v2(handle, command.c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        const std::string error_msg = std::string(sqlite3_errmsg(handle));
        neroshop::log_error("sqlite3_prepare_v2: " + error_msg);
        errors.push_back(std::make_pair(result, error_msg));
        // Since we don't prepare a statement here, there is no need to finalise it
        return result;
    }
    // Bind user-defined parameter arguments
    assert(args.size() == sqlite3_bind_parameter_count(statement));
    for(int i = 0; i < args.size(); i++) {
        result = sqlite3_bind_text(statement, i + 1, args[i].c_str(), args[i].length(), SQLITE_STATIC);
        if(result != SQLITE_OK) {
            const std::string error_msg = std::string(sqlite3_errmsg(handle));
            neroshop::log_error("sqlite3_bind_*: " + error_msg);
            errors.push_back(std::make_pair(result, error_msg));
            sqlite3_finalize(statement);
            return result;
        }
    }
    // Evaluate the statement
    result = sqlite3_step(statement);
    if(result != SQLITE_DONE) {
        const std::string error_msg = std::string(sqlite3_errmsg(handle));
        neroshop::log_error("sqlite3_step: " + error_msg);
        errors.push_back(std::make_pair(result, error_msg));
        sqlite3_finalize(statement);
        return result;
    }    
    // Finalize (destroy) the prepared statement
    result = sqlite3_finalize(statement);
    if(result != SQLITE_OK) {
        const std::string error_msg = std::string(sqlite3_errmsg(handle));
        neroshop::log_error("sqlite3_finalize: " + error_msg);
        errors.push_back(std::make_pair(result, error_msg));
        sqlite3_finalize(statement);
        return result;
    }        
    // Return the result
    return result;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

std::string Sqlite3::get_sqlite_version() {
    return sqlite3_libversion();
}

//-----------------------------------------------------------------------------

sqlite3 * Sqlite3::get_handle() const {
    return handle;
}

//-----------------------------------------------------------------------------

std::string Sqlite3::get_file() const {
    return filename;
}

//-----------------------------------------------------------------------------

std::string Sqlite3::get_name() const {
    if(filename.empty()) return "";
    return filename.substr(filename.find_last_of("\\/") + 1);
}

//-----------------------------------------------------------------------------

std::mutex & Sqlite3::get_mutex() {
    return db_mutex;
}

//-----------------------------------------------------------------------------

void * Sqlite3::get_blob(const std::string& command) {
    if (enable_mutex) std::lock_guard<std::mutex> lock(db_mutex);
    if(!handle) throw std::runtime_error("database is not connected");
    sqlite3_stmt * statement = nullptr;
    int result = sqlite3_prepare_v2(handle, command.c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        const std::string error_msg = std::string(sqlite3_errmsg(handle));
        neroshop::log_error("sqlite3_prepare_v2: " + error_msg);
        errors.push_back(std::make_pair(result, error_msg));
        return nullptr;
    }
    result = sqlite3_step(statement);
    if (result != SQLITE_ROW) {
        const std::string error_msg = std::string(sqlite3_errmsg(handle));
        neroshop::log_error("sqlite3_step: " + error_msg);
        errors.push_back(std::make_pair(result, error_msg));
        sqlite3_finalize(statement);
        return nullptr;
    }
    int column_type = sqlite3_column_type(statement, 0);
    if(column_type == SQLITE_NULL) {
        ////errors.push_back(std::make_pair(, ));
        sqlite3_finalize(statement);
        return nullptr;
    }    
    if(column_type != SQLITE_BLOB) { // NULL is the only other acceptable return type
        neroshop::log_error("sqlite3_column_type: invalid column return type\ncommand: " + command);
        ////errors.push_back(std::make_pair(, ));
        sqlite3_finalize(statement);
        return nullptr;
    }
    void * blob = const_cast<void *>(sqlite3_column_blob(statement, 0));//reinterpret_cast<const char *>(sqlite3_column_text16(stmt, 0)); // utf-16
    sqlite3_finalize(statement);
    return blob;
}

//-----------------------------------------------------------------------------

void * Sqlite3::get_blob_params(const std::string& command, const std::vector<std::string>& args) {
	if (enable_mutex) std::lock_guard<std::mutex> lock(db_mutex);
	if(!handle) throw std::runtime_error("database is not connected");
    sqlite3_stmt * statement = nullptr;
    int result = sqlite3_prepare_v2(handle, command.c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        const std::string error_msg = std::string(sqlite3_errmsg(handle));
        neroshop::log_error("sqlite3_prepare_v2: " + error_msg);
        errors.push_back(std::make_pair(result, error_msg));
        return nullptr;
    }
    // Bind user-defined parameter arguments
    assert(args.size() == sqlite3_bind_parameter_count(statement));
    for(int i = 0; i < args.size(); i++) {
        result = sqlite3_bind_text(statement, i + 1, args[i].c_str(), args[i].length(), SQLITE_STATIC);
        if(result != SQLITE_OK) {
            const std::string error_msg = std::string(sqlite3_errmsg(handle));
            neroshop::log_error("sqlite3_bind_*: " + error_msg);
            errors.push_back(std::make_pair(result, error_msg));
            sqlite3_finalize(statement);
            return nullptr;
        }
    }    
    sqlite3_step(statement); // Don't check for error or it'll keep saying: "another row available" or "no more rows available"
    int column_type = sqlite3_column_type(statement, 0);
    if(column_type == SQLITE_NULL) {
        ////errors.push_back(std::make_pair(, ));
        sqlite3_finalize(statement);
        return nullptr;
    }    
    if(column_type != SQLITE_BLOB) {
        neroshop::log_error("sqlite3_column_type: invalid column return type\ncommand: " + command);
        ////errors.push_back(std::make_pair(, ));
        sqlite3_finalize(statement);
        return nullptr;
    }
    void * blob = const_cast<void *>(sqlite3_column_blob(statement, 0));//reinterpret_cast<const char *>(sqlite3_column_text16(stmt, 0)); // utf-16
    sqlite3_finalize(statement);
    return blob;
}

//-----------------------------------------------------------------------------

std::string Sqlite3::get_text(const std::string& command) {//const {
    if (enable_mutex) std::lock_guard<std::mutex> lock(db_mutex);
    if(!handle) throw std::runtime_error("database is not connected");
    sqlite3_stmt * stmt = nullptr;
    int result = sqlite3_prepare_v2(handle, command.c_str(), -1, &stmt, nullptr);
    if(result != SQLITE_OK) {
        const std::string error_msg = std::string(sqlite3_errmsg(handle));
        neroshop::log_error("sqlite3_prepare_v2: " + error_msg);
        errors.push_back(std::make_pair(result, error_msg));
        return "";
    }
    result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) {
        const std::string error_msg = std::string(sqlite3_errmsg(handle));
        neroshop::log_error("sqlite3_step: " + error_msg);
        errors.push_back(std::make_pair(result, error_msg));
        sqlite3_finalize(stmt);
        return "";
    }
    int column_type = sqlite3_column_type(stmt, 0);
    if(column_type == SQLITE_NULL) {
        ////errors.push_back(std::make_pair(, ));
        sqlite3_finalize(stmt);
        return "";
    }    
    if(column_type != SQLITE_TEXT) {
        neroshop::log_error("sqlite3_column_type: invalid column return type\ncommand: " + command);
        ////errors.push_back(std::make_pair(, ));
        sqlite3_finalize(stmt);
        return "";
    }
    std::string text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));//reinterpret_cast<const char *>(sqlite3_column_text16(stmt, 0)); // utf-16
    sqlite3_finalize(stmt);
    return text;
}

//-----------------------------------------------------------------------------

std::string Sqlite3::get_text_params(const std::string& command, const std::vector<std::string>& args) {//const {
    if (enable_mutex) std::lock_guard<std::mutex> lock(db_mutex);
    if(!handle) throw std::runtime_error("database is not connected");
    // Prepare statement
    sqlite3_stmt * statement = nullptr;
    int result = sqlite3_prepare_v2(handle, command.c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        const std::string error_msg = std::string(sqlite3_errmsg(handle));
        neroshop::log_error("sqlite3_prepare_v2: " + error_msg);
        errors.push_back(std::make_pair(result, error_msg));
        return "";
    }
    // Bind user-defined parameter arguments
    assert(args.size() == sqlite3_bind_parameter_count(statement));
    for(int i = 0; i < args.size(); i++) {
        result = sqlite3_bind_text(statement, i + 1, args[i].c_str(), args[i].length(), SQLITE_STATIC);
        if(result != SQLITE_OK) {
            const std::string error_msg = std::string(sqlite3_errmsg(handle));
            neroshop::log_error("sqlite3_bind_*: " + error_msg);
            errors.push_back(std::make_pair(result, error_msg));
            sqlite3_finalize(statement);
            return "";
        }
    }
    // Evaluate statement
    sqlite3_step(statement); // Don't check for error or it'll keep saying: "another row available" or "no more rows available"
    // Check the type of the statement's return value
    int column_type = sqlite3_column_type(statement, 0);
    if(column_type == SQLITE_NULL) {
        ////errors.push_back(std::make_pair(, ));
        sqlite3_finalize(statement);
        return "";
    }    
    if(column_type != SQLITE_TEXT) {
        neroshop::log_error("sqlite3_column_type: invalid column return type\ncommand: " + command);
        ////errors.push_back(std::make_pair(, ));
        sqlite3_finalize(statement);
        return "";
    }
    // Finalize (destroy) the prepared statement
    std::string text = reinterpret_cast<const char *>(sqlite3_column_text(statement, 0));//reinterpret_cast<const char *>(sqlite3_column_text16(stmt, 0)); // utf-16
    sqlite3_finalize(statement);
    return text;
}

//-----------------------------------------------------------------------------

int Sqlite3::get_integer(const std::string& command) {
    if (enable_mutex) std::lock_guard<std::mutex> lock(db_mutex);
    if(!handle) throw std::runtime_error("database is not connected");
    sqlite3_stmt * statement = nullptr;
    int result = sqlite3_prepare_v2(handle, command.c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        const std::string error_msg = std::string(sqlite3_errmsg(handle));
        neroshop::log_error("sqlite3_prepare_v2: " + error_msg);
        errors.push_back(std::make_pair(result, error_msg));
        return 0;
    }
    result = sqlite3_step(statement);
    if (result != SQLITE_ROW) {
        const std::string error_msg = std::string(sqlite3_errmsg(handle));
        neroshop::log_error("sqlite3_step: " + error_msg);
        errors.push_back(std::make_pair(result, error_msg));
        sqlite3_finalize(statement);
        return 0;
    }
    int column_type = sqlite3_column_type(statement, 0);
    if(column_type == SQLITE_NULL) {
        ////errors.push_back(std::make_pair(, ));
        sqlite3_finalize(statement);
        return 0;
    }    
    if(column_type != SQLITE_INTEGER) {
        neroshop::log_error("sqlite3_column_type: invalid column return type\ncommand: " + command);
        ////errors.push_back(std::make_pair(, ));
        sqlite3_finalize(statement);
        return 0;
    }
    int number = sqlite3_column_int64(statement, 0);//sqlite3_column_int(statement, 0);
    sqlite3_finalize(statement);
    return number;
}

//-----------------------------------------------------------------------------

int Sqlite3::get_integer_params(const std::string& command, const std::vector<std::string>& args) {
    if (enable_mutex) std::lock_guard<std::mutex> lock(db_mutex);
    if(!handle) throw std::runtime_error("database is not connected");
    // Prepare statement
    sqlite3_stmt * statement = nullptr;
    int result = sqlite3_prepare_v2(handle, command.c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        const std::string error_msg = std::string(sqlite3_errmsg(handle));
        neroshop::log_error("sqlite3_prepare_v2: " + error_msg);
        errors.push_back(std::make_pair(result, error_msg));
        return 0;
    }
    // Bind user-defined parameter arguments
    assert(args.size() == sqlite3_bind_parameter_count(statement));
    for(int i = 0; i < args.size(); i++) {
        result = sqlite3_bind_text(statement, i + 1, args[i].c_str(), args[i].length(), SQLITE_STATIC);
        if(result != SQLITE_OK) {
            const std::string error_msg = std::string(sqlite3_errmsg(handle));
            neroshop::log_error("sqlite3_bind_*: " + error_msg);
            errors.push_back(std::make_pair(result, error_msg));
            sqlite3_finalize(statement);
            return 0;
        }
    }
    // Evaluate statement
    sqlite3_step(statement); // Don't check for error or it'll keep saying: "another row available" or "no more rows available"
    // Check the type of the statement's return value
    int column_type = sqlite3_column_type(statement, 0);
    if(column_type == SQLITE_NULL) {
        ////errors.push_back(std::make_pair(, ));
        sqlite3_finalize(statement);
        return 0;
    }    
    if(column_type != SQLITE_INTEGER) {
        neroshop::log_error("sqlite3_column_type: invalid column return type\ncommand: " + command);
        ////errors.push_back(std::make_pair(, ));
        sqlite3_finalize(statement);
        return 0;
    }
    // Finalize (destroy) the prepared statement
    int number = sqlite3_column_int64(statement, 0);//sqlite3_column_int(statement, 0);
    sqlite3_finalize(statement);
    return number;
}

//-----------------------------------------------------------------------------

double Sqlite3::get_real(const std::string& command) {
    if (enable_mutex) std::lock_guard<std::mutex> lock(db_mutex);
    if(!handle) throw std::runtime_error("database is not connected");
    sqlite3_stmt * stmt = nullptr;
    int result = sqlite3_prepare_v2(handle, command.c_str(), -1, &stmt, nullptr);
    if(result != SQLITE_OK) {
        const std::string error_msg = std::string(sqlite3_errmsg(handle));
        neroshop::log_error("sqlite3_prepare_v2: " + error_msg);
        errors.push_back(std::make_pair(result, error_msg));
        return 0.0;
    }
    result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) {
        const std::string error_msg = std::string(sqlite3_errmsg(handle));
        neroshop::log_error("sqlite3_step: " + error_msg);
        errors.push_back(std::make_pair(result, error_msg));
        sqlite3_finalize(stmt);
        return 0.0;
    }
    int column_type = sqlite3_column_type(stmt, 0);
    if(column_type == SQLITE_NULL) {
        ////errors.push_back(std::make_pair(, ));
        sqlite3_finalize(stmt);
        return 0.0;
    }    
    if(column_type != SQLITE_FLOAT) {
        neroshop::log_error("sqlite3_column_type: invalid column return type\ncommand: " + command);
        ////errors.push_back(std::make_pair(, ));
        sqlite3_finalize(stmt);
        return 0.0;
    }
    double number = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
    return number;
}

//-----------------------------------------------------------------------------

double Sqlite3::get_real_params(const std::string& command, const std::vector<std::string>& args) {
    if (enable_mutex) std::lock_guard<std::mutex> lock(db_mutex);
    if(!handle) throw std::runtime_error("database is not connected");
    // Prepare statement
    sqlite3_stmt * statement = nullptr;
    int result = sqlite3_prepare_v2(handle, command.c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        const std::string error_msg = std::string(sqlite3_errmsg(handle));
        neroshop::log_error("sqlite3_prepare_v2: " + error_msg);
        errors.push_back(std::make_pair(result, error_msg));
        return 0.0;
    }
    // Bind user-defined parameter arguments
    assert(args.size() == sqlite3_bind_parameter_count(statement));
    for(int i = 0; i < args.size(); i++) {
        result = sqlite3_bind_text(statement, i + 1, args[i].c_str(), args[i].length(), SQLITE_STATIC);
        if(result != SQLITE_OK) {
            const std::string error_msg = std::string(sqlite3_errmsg(handle));
            neroshop::log_error("sqlite3_bind_*: " + error_msg);
            errors.push_back(std::make_pair(result, error_msg));
            sqlite3_finalize(statement);
            return 0.0;
        }
    }
    // Evaluate statement
    sqlite3_step(statement); // Don't check for error or it'll keep saying: "another row available" or "no more rows available"
    // Check the type of the statement's return value
    int column_type = sqlite3_column_type(statement, 0);
    if(column_type == SQLITE_NULL) {
        sqlite3_finalize(statement);
        ////errors.push_back(std::make_pair(, ));
        return 0.0;
    }    
    if(column_type != SQLITE_FLOAT) {
        neroshop::log_error("sqlite3_column_type: invalid column return type\ncommand: " + command);
        sqlite3_finalize(statement);
        ////errors.push_back(std::make_pair(, ));
        return 0.0;
    }
    // Finalize (destroy) the prepared statement
    double number = sqlite3_column_double(statement, 0);
    sqlite3_finalize(statement);
    return number;
}

//-----------------------------------------------------------------------------

std::vector<std::string> Sqlite3::get_rows(const std::string& command) {
    if (enable_mutex) std::lock_guard<std::mutex> lock(db_mutex);
    if(!handle) throw std::runtime_error("database is not connected");
    sqlite3_stmt * stmt = nullptr;
    std::vector<std::string> row_values = {};
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(handle, command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::log_error("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(handle)));
        return {};
    }
    // Check whether the prepared statement returns no data (for example an UPDATE)
    // "SELECT name FROM users ORDER BY id;"      = 1 column
    // "SELECT name, age FROM users ORDER BY id;" = 2 columns
    // "SELECT * FROM users ORDER BY id;"         = all column(s) including the id
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::log_error("No data found. Be sure to use an appropriate SELECT statement");
        return {};
    }
    // Get all table values row by row instead of column by column
    int result = 0;
    while((result = sqlite3_step(stmt)) == SQLITE_ROW) {
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));////if(sqlite3_column_text(stmt, i) == nullptr) {throw std::runtime_error("column is NULL");}
            row_values.push_back(column_value); //std::cout << sqlite3_column_text(stmt, i) << std::endl;//std::cout << sqlite3_column_name(stmt, i) << std::endl;
            // To get a specific column use: if(i == 0) {} or any number
            // or call sqlite3_column_name for each column
        }
    }

    if(result != SQLITE_DONE) {
        neroshop::log_error("sqlite3_step: " + std::string(sqlite3_errmsg(handle)));
    }
    
    sqlite3_finalize(stmt);
    
    return row_values;
}

//-----------------------------------------------------------------------------

std::pair<int, std::string> Sqlite3::get_error() const {
    if(errors.empty()) return std::make_pair(0, "");
    
    return errors.back();
}

//-----------------------------------------------------------------------------

std::string Sqlite3::get_select() {
    std::string json = json_object.dump();
    json_object.clear();
    return json;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void Sqlite3::set_enable_mutex(bool enabled) {
    enable_mutex = enabled;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool Sqlite3::is_open() const {
    return (opened == true);
}

//-----------------------------------------------------------------------------

bool Sqlite3::table_exists(const std::string& table_name) {
    std::string command = "SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name = $1;";
    return get_integer_params(command, { table_name });
}

//-----------------------------------------------------------------------------

bool Sqlite3::is_mutex_enabled() const {
    return enable_mutex;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

nlohmann::json Sqlite3::json_object;//({});

//-----------------------------------------------------------------------------

int Sqlite3::callback(void *not_used, int argc, char **argv, char **az_col_name)
{
    // Note: This callback is only used when sqlite3_exec / execute() is called and will NOT work with sqlite3_prepare+sqlite3_step+sqlite3_finalize / execute_params
    nlohmann::json row;
    for(int i = 0; i < argc; i++) {
	    row[az_col_name[i]] = argv[i] ? argv[i] : "NULL"; // throws "std::logic_error - what():  basic_string::_M_construct null not valid" exception unless NULL is a string :/
    }
    
    json_object.push_back(row);//std::cout << json_object.dump() << std::endl;
    return 0;
}

//-----------------------------------------------------------------------------

}

}
