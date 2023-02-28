#include "database.hpp"

neroshop::db::Sqlite3 * neroshop::get_database() {
    std::string database_path = NEROSHOP_DEFAULT_DATABASE_PATH;
    std::string database_file = "data.sqlite3";
    static neroshop::db::Sqlite3 database_obj { database_file };////static neroshop::db::Sqlite3 database_obj { database_path + "/" + database_file };
    return &database_obj;
}
