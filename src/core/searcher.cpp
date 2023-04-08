#include "searcher.hpp"
#include "query_builder.hpp"
#include "database.hpp"
#include "debug.hpp"

int neroshop::Searcher::seller_search_total_results(const neroshop::GenericSearchParams& params) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database || !database->get_handle()) throw std::runtime_error("database is NULL");
    int total = 0;
    sqlite3_stmt* statement = nullptr;
    int result = sqlite3_prepare_v2(database->get_handle(), neroshop::QueryBuilder::create_total_results_seller_search_query(params).c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        result = sqlite3_step(statement);
        neroshop::print("Failed to create prepared statement while attempting to find sellers count with error code: "+std::to_string(result)+".", 1);
        return total;
    }
    result = params.bind_params(statement);
    if(result != SQLITE_OK) {
        if(result == SQLITE_ERROR) {
            result = sqlite3_step(statement);
        }
        neroshop::print("Failed to bind parameters while attempting to find sellers count with error code: "+std::to_string(result)+".", 1);
        return total;
    }
    result = sqlite3_step(statement);
    if(result != SQLITE_ROW) {
        result = sqlite3_step(statement);
        neroshop::print("Failed to fully iterate through results while attempting to find sellers count with error code: "+std::to_string(result)+".", 1);
        return total;
    }
    total = sqlite3_column_int(statement, 0);
    if(sqlite3_finalize(statement) != SQLITE_OK) {
        result = sqlite3_step(statement);
        neroshop::print("Failed to finalize prepared statement while attempting to find sellers count with error code: "+std::to_string(result)+".", 1);
        return total;
    }
    return total;
}

std::vector<neroshop::ListingSeller> neroshop::Searcher::seller_search(const neroshop::GenericSearchParams& params) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database || !database->get_handle()) throw std::runtime_error("database is NULL");
    std::vector<neroshop::ListingSeller> sellers;
    sqlite3_stmt* statement = nullptr;
    int result = sqlite3_prepare_v2(database->get_handle(), neroshop::QueryBuilder::create_seller_search_query(params).c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        result = sqlite3_step(statement);
        neroshop::print("Failed to create prepared statement while attempting to find sellers with error code: "+std::to_string(result)+".", 1);
        return sellers;
    }
    result = params.bind_params(statement);
    if(result != SQLITE_OK) {
        if(result == SQLITE_ERROR) {
            result = sqlite3_step(statement);
        }
        neroshop::print("Failed to bind parameters while attempting to find sellers with error code: "+std::to_string(result)+".", 1);
        return sellers;
    }
    int iterations = 0;
    result = sqlite3_step(statement);
    do {
        if(iterations > ITERATION_LIMIT) {
            break;
        }
        if(result != SQLITE_ROW && result != SQLITE_DONE) {
            result = sqlite3_step(statement);
            neroshop::print("Failed to fully iterate through results while attempting to find sellers with error code: "+std::to_string(result)+".", 1);
            return sellers;
        }
        if(result == SQLITE_DONE) {
            break;
        }
        sellers.push_back(neroshop::ListingSeller(statement));
        result = sqlite3_step(statement);
        ++iterations;
    } while(result != SQLITE_DONE);
    if(sqlite3_finalize(statement) != SQLITE_OK) {
        result = sqlite3_step(statement);
        neroshop::print("Failed to finalize prepared statement while attempting to find sellers with error code: "+std::to_string(result)+".", 1);
        return sellers;
    }
    return sellers;
}

std::vector<neroshop::ListingSeller> neroshop::Searcher::find_seller_by_id(int seller_id) {
    neroshop::GenericSearchParams params = neroshop::GenericSearchParams();
    params.id = seller_id;
    return neroshop::Searcher::seller_search(params);
}

int neroshop::Searcher::seller_search_by_name_total_results(std::string name) {
    neroshop::GenericSearchParams params = neroshop::GenericSearchParams();
    params.name = name;
    return neroshop::Searcher::seller_search_total_results(params);
}

std::vector<neroshop::ListingSeller> neroshop::Searcher::seller_search_by_name(std::string name, int page, int count) {
    neroshop::GenericSearchParams params = neroshop::GenericSearchParams();
    params.name = name;
    params.page = page;
    params.count = count;
    return neroshop::Searcher::seller_search(params);
}

int neroshop::Searcher::product_search_total_results(const neroshop::ProductSearchParams& params) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database || !database->get_handle()) throw std::runtime_error("database is NULL");
    int total = 0;
    sqlite3_stmt* statement = nullptr;
    int result = sqlite3_prepare_v2(database->get_handle(), neroshop::QueryBuilder::create_total_results_product_search_query(params).c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        result = sqlite3_step(statement);
        neroshop::print("Failed to create prepared statement while attempting to find products count with error code: "+std::to_string(result)+".", 1);
        return total;
    }
    result = params.bind_params(statement);
    if(result != SQLITE_OK) {
        if(result == SQLITE_ERROR) {
            result = sqlite3_step(statement);
        }
        neroshop::print("Failed to bind parameters while attempting to find products count with error code: "+std::to_string(result)+".", 1);
        return total;
    }
    result = sqlite3_step(statement);
    if(result != SQLITE_ROW) {
        result = sqlite3_step(statement);
        neroshop::print("Failed to fully iterate through results while attempting to find products count with error code: "+std::to_string(result)+".", 1);
        return total;
    }
    total = sqlite3_column_int(statement, 0);
    if(sqlite3_finalize(statement) != SQLITE_OK) {
        result = sqlite3_step(statement);
        neroshop::print("Failed to finalize prepared statement while attempting to find products count with error code: "+std::to_string(result)+".", 1);
        return total;
    }
    return total;
}

std::vector<neroshop::Product> neroshop::Searcher::product_search(const neroshop::ProductSearchParams& params) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database || !database->get_handle()) throw std::runtime_error("database is NULL");
    std::vector<neroshop::Product> products;
    sqlite3_stmt* statement = nullptr;
    int result = sqlite3_prepare_v2(database->get_handle(), neroshop::QueryBuilder::create_product_search_query(params).c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        result = sqlite3_step(statement);
        neroshop::print("Failed to create prepared statement while attempting to find products with error code: "+std::to_string(result)+".", 1);
        return products;
    }
    result = params.bind_params(statement);
    if(result != SQLITE_OK) {
        if(result == SQLITE_ERROR) {
            result = sqlite3_step(statement);
        }
        neroshop::print("Failed to bind parameters while attempting to find products with error code: "+std::to_string(result)+".", 1);
        return products;
    }
    int iterations = 0;
    result = sqlite3_step(statement);
    do {
        if(iterations > ITERATION_LIMIT) {
            break;
        }
        if(result != SQLITE_ROW && result != SQLITE_DONE) {
            result = sqlite3_step(statement);
            neroshop::print("Failed to fully iterate through results while attempting to find products with error code: "+std::to_string(result)+".", 1);
            return products;
        }
        if(result == SQLITE_DONE) {
            break;
        }
        products.push_back(neroshop::Product(statement));
        result = sqlite3_step(statement);
        ++iterations;
    } while(result != SQLITE_DONE);
    if(sqlite3_finalize(statement) != SQLITE_OK) {
        result = sqlite3_step(statement);
        neroshop::print("Failed to finalize prepared statement while attempting to find products with error code: "+std::to_string(result)+".", 1);
        return products;
    }
    return products;
}

std::vector<neroshop::Product> neroshop::Searcher::product_search_by_id(int product_id) {
    neroshop::ProductSearchParams params = neroshop::ProductSearchParams();
    params.id = product_id;
    return neroshop::Searcher::product_search(params);
}

int neroshop::Searcher::product_search_by_code_total_results(std::string code) {
    neroshop::ProductSearchParams params = neroshop::ProductSearchParams();
    params.code = code;
    return neroshop::Searcher::product_search_total_results(params);
}

std::vector<neroshop::Product> neroshop::Searcher::product_search_by_code(std::string code, int page, int count) {
    neroshop::ProductSearchParams params = neroshop::ProductSearchParams();
    params.code = code;
    params.page = page;
    params.count = count;
    return neroshop::Searcher::product_search(params);
}

int neroshop::Searcher::product_search_by_category_id_total_results(int category_id) {
    neroshop::ProductSearchParams params = neroshop::ProductSearchParams();
    params.category_id = category_id;
    return neroshop::Searcher::product_search_total_results(params);
}

std::vector<neroshop::Product> neroshop::Searcher::product_search_by_category_id(int category_id, int page, int count) {
    neroshop::ProductSearchParams params = neroshop::ProductSearchParams();
    params.category_id = category_id;
    params.page = page;
    params.count = count;
    return neroshop::Searcher::product_search(params);
}

int neroshop::Searcher::product_search_by_highest_rating_total_results() {
    neroshop::ProductSearchParams params = neroshop::ProductSearchParams();
    params.order_by_column = STRINGIFY(PRODUCT_RATING_COLUMN_NAME);
    params.order_by_ascending = false;
    return neroshop::Searcher::product_search_total_results(params);
}

std::vector<neroshop::Product> neroshop::Searcher::product_search_by_highest_rating(int page, int count) {
    neroshop::ProductSearchParams params = neroshop::ProductSearchParams();
    params.page = page;
    params.count = count;
    params.order_by_column = STRINGIFY(PRODUCT_RATING_COLUMN_NAME);
    params.order_by_ascending = false;
    return neroshop::Searcher::product_search(params);
}

int neroshop::Searcher::product_search_by_most_rated_total_results() {
    neroshop::ProductSearchParams params = neroshop::ProductSearchParams();
    params.order_by_column = STRINGIFY(PRODUCT_RATING_COUNT_COLUMN_NAME);
    params.order_by_ascending = false;
    return neroshop::Searcher::product_search_total_results(params);
}

std::vector<neroshop::Product> neroshop::Searcher::product_search_by_most_rated(int page, int count) {
    neroshop::ProductSearchParams params = neroshop::ProductSearchParams();
    params.page = page;
    params.count = count;
    params.order_by_column = STRINGIFY(PRODUCT_RATING_COUNT_COLUMN_NAME);
    params.order_by_ascending = false;
    return neroshop::Searcher::product_search(params);
}

int neroshop::Searcher::listing_search_total_results(const neroshop::ListingSearchParams& params) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database || !database->get_handle()) throw std::runtime_error("database is NULL");
    int total = 0;
    sqlite3_stmt* statement = nullptr;
    int result = sqlite3_prepare_v2(database->get_handle(), neroshop::QueryBuilder::create_total_results_listing_search_query(params).c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        result = sqlite3_step(statement);
        neroshop::print("Failed to create prepared statement while attempting to find listings count with error code: "+std::to_string(result)+".", 1);
        return total;
    }
    result = params.bind_params(statement);
    if(result != SQLITE_OK) {
        if(result == SQLITE_ERROR) {
            result = sqlite3_step(statement);
        }
        neroshop::print("Failed to bind parameters while attempting to find listings count with error code: "+std::to_string(result)+".", 1);
        return total;
    }
    result = sqlite3_step(statement);
    if(result != SQLITE_ROW) {
        result = sqlite3_step(statement);
        neroshop::print("Failed to fully iterate through results while attempting to find listings count with error code: "+std::to_string(result)+".", 1);
        return total;
    }
    total = sqlite3_column_int(statement, 0);
    if(sqlite3_finalize(statement) != SQLITE_OK) {
        result = sqlite3_step(statement);
        neroshop::print("Failed to finalize prepared statement while attempting to find listings count with error code: "+std::to_string(result)+".", 1);
        return total;
    }
    return total;
}

std::vector<neroshop::Listing> neroshop::Searcher::listing_search(const neroshop::ListingSearchParams& params) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database || !database->get_handle()) throw std::runtime_error("database is NULL");
    std::vector<neroshop::Listing> listings;
    sqlite3_stmt* statement = nullptr;
    int result = sqlite3_prepare_v2(database->get_handle(), neroshop::QueryBuilder::create_listing_search_query(params).c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        result = sqlite3_step(statement);
        neroshop::print("Failed to create prepared statement while attempting to find listings with error code: "+std::to_string(result)+".", 1);
        return listings;
    }
    result = params.bind_params(statement);
    if(result != SQLITE_OK) {
        if(result == SQLITE_ERROR) {
            result = sqlite3_step(statement);
        }
        neroshop::print("Failed to bind parameters while attempting to find listings with error code: "+std::to_string(result)+".", 1);
        return listings;
    }
    int iterations = 0;
    result = sqlite3_step(statement);
    do {
        if(iterations > ITERATION_LIMIT) {
            break;
        }
        if(result != SQLITE_ROW && result != SQLITE_DONE) {
            result = sqlite3_step(statement);
            neroshop::print("Failed to fully iterate through results while attempting to find listings with error code: "+std::to_string(result)+".", 1);
            return listings;
        }
        if(result == SQLITE_DONE) {
            break;
        }
        listings.push_back(neroshop::Listing(statement));
        result = sqlite3_step(statement);
        ++iterations;
    } while(result != SQLITE_DONE);
    if(sqlite3_finalize(statement) != SQLITE_OK) {
        result = sqlite3_step(statement);
        neroshop::print("Failed to finalize prepared statement while attempting to find listings with error code: "+std::to_string(result)+".", 1);
        return listings;
    }
    return listings;
}

int neroshop::Searcher::listing_search_by_product_id_total_results(std::string product_id) {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.product_id = product_id;
    return neroshop::Searcher::listing_search_total_results(params);
}

std::vector<neroshop::Listing> neroshop::Searcher::listing_search_by_product_id(std::string product_id, int page, int count) {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.product_id = product_id;
    params.page = page;
    params.count = count;
    return neroshop::Searcher::listing_search(params);
}

int neroshop::Searcher::listing_search_by_seller_id_total_results(std::string seller_id) {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.seller_id = seller_id;
    return neroshop::Searcher::listing_search_total_results(params);
}

std::vector<neroshop::Listing> neroshop::Searcher::listing_search_by_seller_id(std::string seller_id, int page, int count) {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.seller_id = seller_id;
    params.page = page;
    params.count = count;
    return neroshop::Searcher::listing_search(params);
}

int neroshop::Searcher::listing_search_by_seller_total_results(ListingSeller seller) {
    return neroshop::Searcher::listing_search_by_seller_id_total_results(seller.get_id());
}

std::vector<neroshop::Listing> neroshop::Searcher::listing_search_by_seller(ListingSeller seller, int page, int count) {
    return neroshop::Searcher::listing_search_by_seller_id(seller.get_id(), page, count);
}

int neroshop::Searcher::listing_search_by_category_id_total_results(int category_id) {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.category_id = category_id;
    return neroshop::Searcher::listing_search_total_results(params);
}

std::vector<neroshop::Listing> neroshop::Searcher::listing_search_by_category_id(int category_id, int page, int count) {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.category_id = category_id;
    params.page = page;
    params.count = count;
    return neroshop::Searcher::listing_search(params);
}

int neroshop::Searcher::listing_search_by_most_recent_total_results() {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.order_by_column = STRINGIFY(LISTING_DATE_COLUMN_NAME);
    params.order_by_ascending = false;
    return neroshop::Searcher::listing_search_total_results(params);
}

std::vector<neroshop::Listing> neroshop::Searcher::listing_search_by_most_recent(int page, int count) {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.page = page;
    params.count = count;
    params.order_by_column = STRINGIFY(LISTING_DATE_COLUMN_NAME);
    params.order_by_ascending = false;
    return neroshop::Searcher::listing_search(params);
}

int neroshop::Searcher::listing_search_by_oldest_total_results() {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.order_by_column = STRINGIFY(LISTING_DATE_COLUMN_NAME);
    params.order_by_ascending = true;
    return neroshop::Searcher::listing_search_total_results(params);
}

std::vector<neroshop::Listing> neroshop::Searcher::listing_search_by_oldest(int page, int count) {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.page = page;
    params.count = count;
    params.order_by_column = STRINGIFY(LISTING_DATE_COLUMN_NAME);
    params.order_by_ascending = true;
    return neroshop::Searcher::listing_search(params);
}

int neroshop::Searcher::listing_search_by_alphabetical_order_total_results() {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.order_by_column = STRINGIFY(PRODUCT_TABLE_NAME) "." STRINGIFY(PRODUCT_NAME_COLUMN_NAME);
    params.order_by_ascending = true;
    params.collate_order_by = true;
    params.no_case_order_by = true;
    return neroshop::Searcher::listing_search_total_results(params);
}

std::vector<neroshop::Listing> neroshop::Searcher::listing_search_by_alphabetical_order(int page, int count) {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.page = page;
    params.count = count;
    params.order_by_column = STRINGIFY(PRODUCT_TABLE_NAME) "." STRINGIFY(PRODUCT_NAME_COLUMN_NAME);
    params.order_by_ascending = true;
    params.collate_order_by = true;
    params.no_case_order_by = true;
    return neroshop::Searcher::listing_search(params);
}

int neroshop::Searcher::listing_search_by_price_lowest_total_results() {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.order_by_column = STRINGIFY(LISTING_PRICE_COLUMN_NAME);
    params.order_by_ascending = true;
    return neroshop::Searcher::listing_search_total_results(params);
}

std::vector<neroshop::Listing> neroshop::Searcher::listing_search_by_price_lowest(int page, int count) {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.page = page;
    params.count = count;
    params.order_by_column = STRINGIFY(LISTING_PRICE_COLUMN_NAME);
    params.order_by_ascending = true;
    return neroshop::Searcher::listing_search(params);
}

int neroshop::Searcher::listing_search_by_price_highest_total_results() {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.order_by_column = STRINGIFY(LISTING_PRICE_COLUMN_NAME);
    params.order_by_ascending = false;
    return neroshop::Searcher::listing_search_total_results(params);
}

std::vector<neroshop::Listing> neroshop::Searcher::listing_search_by_price_highest(int page, int count) {
    neroshop::ListingSearchParams params = neroshop::ListingSearchParams();
    params.page = page;
    params.count = count;
    params.order_by_column = STRINGIFY(LISTING_PRICE_COLUMN_NAME);
    params.order_by_ascending = false;
    return neroshop::Searcher::listing_search(params);
}
