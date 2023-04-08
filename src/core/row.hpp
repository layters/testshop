#ifndef ROW_HPP_NEROSHOP
#define ROW_HPP_NEROSHOP

#include <string>
#include <vector>
#include "sqlite3.h"

#define DECLARE_COLUMN_NAMES_MEMBERS(table) \
static std::vector<std::string> column_names; \
static std::string get_table_names_for_query(bool prefix_table = true) { \
    std::string query_string(""); \
    for(int i = 0; i < column_names.size(); i++) { \
        if(prefix_table) { \
            query_string += std::string(STRINGIFY(table)) + std::string(".");\
        } \
        query_string += column_names[i]; \
        query_string += std::string(" AS ")+std::string(STRINGIFY(table)) + std::string("_")+column_names[i]; \
        if(i < column_names.size()-1) { \
            query_string += ", "; \
        } \
    } \
    return query_string; \
}

#define FILL_COLUMN_NAMES(ClassName, ...) \
std::vector<std::string> ClassName::column_names = { __VA_ARGS__ };

#define STRINGIFY(x) STRINGIFY_E(x)
#define STRINGIFY_E(x) #x

namespace neroshop {
class Row {
public:
    virtual void set_member_from_column_number(sqlite3_stmt* statement, const int column_number) = 0;

    void initialize_from_statement(sqlite3_stmt* statement);
};
}
#endif
