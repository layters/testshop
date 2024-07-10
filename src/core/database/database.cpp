#include "database.hpp"

#include "../../neroshop_config.hpp"

namespace neroshop {

db::Sqlite3 * get_database() {
    std::string database_path = NEROSHOP_DEFAULT_DATABASE_PATH;
    std::string database_file = NEROSHOP_DATABASE_FILENAME;
    static db::Sqlite3 database_obj { database_path + "/" + database_file };
    return &database_obj;
}

db::Sqlite3 * get_user_database() {
    std::string database_path = NEROSHOP_DEFAULT_DATABASE_PATH;
    std::string user_database_file = NEROSHOP_USERDATA_FILENAME;
    static db::Sqlite3 user_database_obj { database_path + "/" + user_database_file };
    return &user_database_obj;
}

}
