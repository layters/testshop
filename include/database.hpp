#ifndef DATABASE_HPP_NEROSHOP // recommended to add unique identifier like _NEROSHOP to avoid naming collision with other libraries
#define DATABASE_HPP_NEROSHOP

//#include "database/lmdb.hpp"
#if defined(NEROSHOP_USE_POSTGRESQL) 
#include "database/postgresql.hpp"
#endif
#include "database/sqlite.hpp"

#endif
