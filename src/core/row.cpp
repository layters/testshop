#include "row.hpp"
#include "debug.hpp"

void neroshop::Row::initialize_from_statement(sqlite3_stmt* statement) {
    if(!statement) {
        neroshop::print("Invalid sqlite3_stmt passed to initializeFromStatement.", 2);
        return;
    }
    const int column_count = sqlite3_column_count(statement);
    for(int column_number = 0; column_number < column_count; column_number++) {
        set_member_from_column_number(statement, column_number);
    }
}
