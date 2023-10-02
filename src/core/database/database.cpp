#include "database.hpp"

#include "../../neroshop_config.hpp"

neroshop::db::Sqlite3 * neroshop::get_database() {
    std::string database_path = NEROSHOP_DEFAULT_DATABASE_PATH;
    std::string database_file = NEROSHOP_DATABASE_FILENAME;
    static neroshop::db::Sqlite3 database_obj { database_path + "/" + database_file };
    return &database_obj;
}
