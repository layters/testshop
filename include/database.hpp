#ifndef DATABASE_HPP_NEROSHOP // recommended to add unique identifier like _NEROSHOP to avoid naming collision with other libraries
#define DATABASE_HPP_NEROSHOP

//#include "" // non-relational or NoSQL
#if defined(NEROSHOP_USE_POSTGRESQL) 
#include "database/postgresql.hpp" // relational (server-based)
#endif
#include "database/sqlite.hpp" // relational (file-based)

#endif
