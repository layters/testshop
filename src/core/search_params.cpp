#include "search_params.hpp"

int neroshop::GenericSearchParams::bind_params(sqlite3_stmt* statement) const {
    std::vector<std::string> terms = neroshop::string::split(neroshop::string::trim(this->terms), " ");
    int result = SQLITE_OK;
    int i = 0;
    for(std::string term : terms) {
        term = std::string("%")+term+std::string("%");
        result = sqlite3_bind_text(statement, ++i, term.c_str(), -1, SQLITE_TRANSIENT);
        if(result != SQLITE_OK) {
            return result;
        }
        result = sqlite3_bind_text(statement, ++i, term.c_str(), -1, SQLITE_TRANSIENT);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    if(id.length() > 0) {
        result = sqlite3_bind_text(statement, ++i, id.c_str(), -1, SQLITE_STATIC);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    if(name.length() > 0) {
        result = sqlite3_bind_text(statement, ++i, name.c_str(), -1, SQLITE_STATIC);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    return result;
}

int neroshop::ProductSearchParams::bind_params(sqlite3_stmt* statement) const {
    int result = GenericSearchParams::bind_params(statement);
    if(result != SQLITE_OK) {
        return result;
    }
    int i = (2*neroshop::string::split(neroshop::string::trim(this->terms), " ").size())
        +(id.length() > 0)
        +(name.length() > 0);
    if(min_weight > 0) {
        result = sqlite3_bind_double(statement, ++i, min_weight);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    if(max_weight > 0) {
        result = sqlite3_bind_double(statement, ++i, max_weight);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    if(attributes.length() > 0) {
        std::string attributes_with_wildcards = std::string("%")+attributes+std::string("%");
        result = sqlite3_bind_text(statement, ++i, attributes_with_wildcards.c_str(), -1, SQLITE_TRANSIENT);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    if(code.length() > 0) {
        result = sqlite3_bind_text(statement, ++i, code.c_str(), -1, SQLITE_STATIC);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    if(category_id > 0) {
        result = sqlite3_bind_int(statement, ++i, category_id);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    if(location.length() > 0) {
        result = sqlite3_bind_text(statement, ++i, location.c_str(), -1, SQLITE_STATIC);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    if(date.length() > 0) {
        result = sqlite3_bind_text(statement, ++i, date.c_str(), -1, SQLITE_STATIC);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    return result;
}

int neroshop::ListingSearchParams::bind_params(sqlite3_stmt* statement) const {
    int result = ProductSearchParams::bind_params(statement);
    if(result != SQLITE_OK) {
        return result;
    }
    int i = (2*neroshop::string::split(neroshop::string::trim(this->terms), " ").size())
        +(id.length() > 0)
        +(name.length() > 0)
        +(min_weight > 0)
        +(max_weight > 0)
        +(attributes.length() > 0)
        +(code.length() > 0)
        +(category_id > 0);
    if(product_id.length() > 0) {
        result = sqlite3_bind_text(statement, ++i, product_id.c_str(), -1, SQLITE_STATIC);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    if(seller_id.length() > 0) {
        result = sqlite3_bind_text(statement, ++i, seller_id.c_str(), -1, SQLITE_STATIC);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    if(quantity > 0) {
        result = sqlite3_bind_int(statement, ++i, quantity);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    if(min_price > 0) {
        result = sqlite3_bind_double(statement, ++i, min_price);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    if(max_price > 0) {
        result = sqlite3_bind_double(statement, ++i, max_price);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    if(currency.length() > 0) {
        result = sqlite3_bind_text(statement, ++i, currency.c_str(), -1, SQLITE_STATIC);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    if(condition.length() > 0) {
        result = sqlite3_bind_text(statement, ++i, condition.c_str(), -1, SQLITE_STATIC);
        if(result != SQLITE_OK) {
            return result;
        }
    }
    return result;
}
