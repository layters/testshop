#include "database.hpp"

#include "../../neroshop_config.hpp"

namespace neroshop {

db::Sqlite3 * get_database() {
    std::string database_path = neroshop::get_default_database_path();
    std::string database_file = NEROSHOP_DATABASE_FILENAME;
    // Clients must NEVER write to the main database at any moment!!
    #if defined(NEROSHOP_BUILD_GUI) || defined(NEROSHOP_BUILD_CLI)
    static db::Sqlite3 database_obj { database_path + "/" + database_file, SQLITE_OPEN_READONLY };
    #else
    static db::Sqlite3 database_obj { database_path + "/" + database_file };
    #endif
    return &database_obj;
}

db::Sqlite3 * get_client_database() {
    std::string database_path = neroshop::get_default_database_path();
    std::string user_database_file = NEROSHOP_USERDATA_FILENAME;
    static db::Sqlite3 user_database_obj { database_path + "/" + user_database_file };
    return &user_database_obj;
}

}
