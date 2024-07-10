#pragma once
#ifndef DATABASE_HPP_NEROSHOP
#define DATABASE_HPP_NEROSHOP

#include "sqlite3/sqlite3.hpp"

namespace neroshop {

db::Sqlite3 * get_database();
db::Sqlite3 * get_user_database();

}
#endif
