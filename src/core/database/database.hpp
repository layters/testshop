#pragma once
#ifndef DATABASE_HPP_NEROSHOP
#define DATABASE_HPP_NEROSHOP

#include "../../neroshop_config.hpp"

#include "sqlite.hpp"

namespace neroshop {

db::Sqlite3 * get_database();

}
#endif
