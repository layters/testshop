#include <iostream>
#include <string>
// neroshop
#include "../include/neroshop.hpp"
using namespace neroshop;
// dokun-ui
/*#include <dokun_ui.hpp>
using namespace dokun;*/

lua_State * neroshop::lua_state = luaL_newstate(); // lua_state should be initialized by default
int main() {
    // Do some testing here ...
    std::cout << "sqlite3 v" << DB::SQLite3::get_sqlite_version() << std::endl;
    std::unique_ptr<DB::SQLite3> database = std::make_unique<DB::SQLite3>();//new DB::SQLite3();
    if(!database->open("data.sqlite3")) {//(":memory:")) { // In-memory databases are temporary and written in RAM
        neroshop::print(SQLITE3_TAG "\033[91mSQLite::open failed");
    }
    //-------------------------
    // Create a table and add columns
    if(!database->table_exists("users")) {
        std::cout << "CREATE TABLE users\n";
        database->execute("CREATE TABLE IF NOT EXISTS users(id  INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        database->execute("ALTER TABLE users ADD name text;");
        //database->execute("ALTER TABLE users ADD pw_hash text;");
        database->execute("ALTER TABLE users ADD age integer;");
    }
    //database->execute();
    // Insert a new row
    ////database->execute_params("INSERT INTO users (name, age) VALUES ($1, $2);", { "sid", "25" });
    std::cout << database->get_text_params("SELECT name FROM users WHERE id = $1", { "1" }) << std::endl;
    // Modify new row
    //database->execute_params("SELECT * FROM users WHERE id = $1 AND name = $2", { "1", "dude" });
    //-------------------------
    // Close database when app is terminated
    database->close();
    return 0;
}
