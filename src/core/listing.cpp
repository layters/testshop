#include <cstring>
#include "debug.hpp"
#include "listing.hpp"

neroshop::Listing::Listing() : id(""), product(neroshop::Product()), seller(neroshop::ListingSeller()), quantity(-1), price(-1), currency(""), condition("") {
}

neroshop::Listing::Listing(sqlite3_stmt* statement) : neroshop::Listing() {
    initialize_from_statement(statement);
    product = neroshop::Product(statement);
    seller = neroshop::ListingSeller(statement);
}

void neroshop::Listing::set_member_from_column_number(sqlite3_stmt* statement, const int column_number) {
    const char* column_name = sqlite3_column_name(statement, column_number);
    if(!strcmp(STRINGIFY(LISTING_TABLE_NAME) "_" STRINGIFY(LISTING_ID_COLUMN_NAME), column_name)) {
        set_id(reinterpret_cast<const char*>(sqlite3_column_text(statement, column_number)));
    } else if(!strcmp(STRINGIFY(LISTING_TABLE_NAME) "_" STRINGIFY(LISTING_QUANTITY_COLUMN_NAME), column_name)) {
        set_quantity(sqlite3_column_int(statement, column_number));
    } else if(!strcmp(STRINGIFY(LISTING_TABLE_NAME) "_" STRINGIFY(LISTING_PRICE_COLUMN_NAME), column_name)) {
        set_price(sqlite3_column_double(statement, column_number));
    } else if(!strcmp(STRINGIFY(LISTING_TABLE_NAME) "_" STRINGIFY(LISTING_CURRENCY_COLUMN_NAME), column_name)) {
        set_currency(sqlite3_column_text(statement, column_number));
    } else if(!strcmp(STRINGIFY(LISTING_TABLE_NAME) "_" STRINGIFY(LISTING_CONDITION_COLUMN_NAME), column_name)) {
        set_condition(sqlite3_column_text(statement, column_number));
    } else if(!strcmp(STRINGIFY(LISTING_TABLE_NAME) "_" STRINGIFY(LISTING_LOCATION_COLUMN_NAME), column_name)) {
        set_location(sqlite3_column_text(statement, column_number));
    } else if(!strcmp(STRINGIFY(LISTING_TABLE_NAME) "_" STRINGIFY(LISTING_DATE_COLUMN_NAME), column_name)) {
        set_date(sqlite3_column_text(statement, column_number));
    } else {
        if(!strcmp(STRINGIFY(LISTING_TABLE_NAME), column_name)) {
            neroshop::print(std::string("Unexpected column name '")+column_name+std::string("' encountered while creating Listing object."), 2);
        }
    }
}

std::string neroshop::Listing::to_string() const {
    std::string str = "(Listing> ";
    str += std::string("Id:")+get_id();
    str += std::string(", Product:")+get_product();
    str += std::string(", Seller:")+get_seller();
    str += std::string(", Quantity:")+std::to_string(get_quantity());
    str += std::string(", Price:")+std::to_string(get_price());
    str += std::string(", Currency:")+get_currency();
    str += std::string(", Condition:")+get_condition();
    str += std::string(", Location:")+get_location();
    str += std::string(", Date:")+get_date();
    str += ")";
    return str;
}

void neroshop::Listing::set_id(std::string new_id) {
    id = new_id;
}

void neroshop::Listing::set_product(neroshop::Product new_product) {
    product = new_product;
}

void neroshop::Listing::set_seller(neroshop::ListingSeller new_seller) {
    seller = new_seller;
}

void neroshop::Listing::set_quantity(int new_quantity) {
    quantity = new_quantity;
}

void neroshop::Listing::set_price(double new_price) {
    price = new_price;
}

void neroshop::Listing::set_currency(std::string new_currency) {
    currency = new_currency;
}

void neroshop::Listing::set_currency(const unsigned char* new_currency) {
    std::string new_value = "NULL";
    if(new_currency) {
        new_value = reinterpret_cast<const char*>(new_currency);
    }
    set_currency(new_value);
}

void neroshop::Listing::set_condition(std::string new_condition) {
    condition = new_condition;
}

void neroshop::Listing::set_condition(const unsigned char* new_condition) {
    std::string new_value = "NULL";
    if(new_condition) {
        new_value = reinterpret_cast<const char*>(new_condition);
    }
    set_condition(new_value);
}

void neroshop::Listing::set_location(std::string new_location) {
    location = new_location;
}

void neroshop::Listing::set_location(const unsigned char* new_location) {
    std::string new_value = "NULL";
    if(new_location) {
        new_value = reinterpret_cast<const char*>(new_location);
    }
    set_location(new_value);
}

void neroshop::Listing::set_date(std::string new_date) {
    date = new_date;
}

void neroshop::Listing::set_date(const unsigned char* new_date) {
    std::string new_value = "NULL";
    if(new_date) {
        new_value = reinterpret_cast<const char*>(new_date);
    }
    set_date(new_value);
}

std::string neroshop::Listing::get_id() const {
    return id;
}

neroshop::Product neroshop::Listing::get_product() const {
    return product;
}

neroshop::ListingSeller neroshop::Listing::get_seller() const {
    return seller;
}

int neroshop::Listing::get_quantity() const {
    return quantity;
}

double neroshop::Listing::get_price() const {
    return price;
}

std::string neroshop::Listing::get_currency() const {
    return currency;
}

std::string neroshop::Listing::get_condition() const {
    return condition;
}

std::string neroshop::Listing::get_location() const {
    return location;
}

std::string neroshop::Listing::get_date() const {
    return date;
}

#if defined(NEROSHOP_BUILD_GUI)
QVariantMap neroshop::Listing::as_q_map() {
    QVariantMap out;
    out.insert("listing_uuid", QString::fromStdString(get_id()));
    out.insert("product_id", QString::fromStdString(get_product().get_id()));
    out.insert("seller_id", QString::fromStdString(get_seller().get_id()));
    out.insert("quantity", get_quantity());
    out.insert("price", get_price());
    out.insert("currency", QString::fromStdString(get_currency()));
    out.insert("condition", QString::fromStdString(get_condition()));
    out.insert("location", QString::fromStdString(get_location()));
    out.insert("date", QString::fromStdString(get_date()));
    out.insert("product_uuid", QString::fromStdString(get_product().get_id()));
    out.insert("product_name", QString::fromStdString(get_product().get_name()));
    out.insert("product_description", QString::fromStdString(get_product().get_description()));
    out.insert("weight", get_product().get_weight());
    out.insert("product_attributes", QString::fromStdString(get_product().get_attributes()));
    out.insert("product_code", QString::fromStdString(get_product().get_code()));
    out.insert("product_category_id", get_product().get_category_id());;
    out.insert("product_code", QString::fromStdString(get_product().get_code()));
    out.insert("product_category_id", get_product().get_category_id());
    out.insert("product_rating", get_product().get_rating());
    out.insert("product_rating_count", get_product().get_rating_count());
    return out;
}
#endif

std::string neroshop::operator+(std::string str, const neroshop::Listing& listing) {
    return str+listing.to_string();
}

std::string neroshop::operator+(const neroshop::Listing& listing, std::string str) {
    return listing.to_string()+str;
}

FILL_COLUMN_NAMES(neroshop::Listing, LISTING_COLUMN_NAME_ARRAY)
