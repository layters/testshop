#include <cstring>
#include "debug.hpp"
#include "product.hpp"

neroshop::Product::Product() : id(""), name(""), description(""), weight(-1), attributes(""), code(""), category_id(-1), rating(-1), rating_count(-1) {
}

neroshop::Product::Product(sqlite3_stmt* statement) : neroshop::Product() {
    initialize_from_statement(statement);
}

void neroshop::Product::set_member_from_column_number(sqlite3_stmt* statement, const int column_number) {
    const char* column_name = sqlite3_column_name(statement, column_number);
    if(!strcmp(STRINGIFY(PRODUCT_TABLE_NAME) "_" STRINGIFY(PRODUCT_ID_COLUMN_NAME), column_name)) {
        set_id(sqlite3_column_text(statement, column_number));
    } else if(!strcmp(STRINGIFY(PRODUCT_TABLE_NAME) "_" STRINGIFY(PRODUCT_NAME_COLUMN_NAME), column_name)) {
        set_name(sqlite3_column_text(statement, column_number));
    } else if(!strcmp(STRINGIFY(PRODUCT_TABLE_NAME) "_" STRINGIFY(PRODUCT_DESCRIPTION_COLUMN_NAME), column_name)) {
        set_description(sqlite3_column_text(statement, column_number));
    } else if(!strcmp(STRINGIFY(PRODUCT_TABLE_NAME) "_" STRINGIFY(PRODUCT_WEIGHT_COLUMN_NAME), column_name)) {
        set_weight(sqlite3_column_double(statement, column_number));
    } else if(!strcmp(STRINGIFY(PRODUCT_TABLE_NAME) "_" STRINGIFY(PRODUCT_ATTRIBUTES_COLUMN_NAME), column_name)) {
        set_attributes(sqlite3_column_text(statement, column_number));
    } else if(!strcmp(STRINGIFY(PRODUCT_TABLE_NAME) "_" STRINGIFY(PRODUCT_CODE_COLUMN_NAME), column_name)) {
        set_code(sqlite3_column_text(statement, column_number));
    } else if(!strcmp(STRINGIFY(PRODUCT_TABLE_NAME) "_" STRINGIFY(PRODUCT_CATEGORY_ID_COLUMN_NAME), column_name)) {
        set_category_id(sqlite3_column_int(statement, column_number));
    } else if(!strcmp(STRINGIFY(PRODUCT_TABLE_NAME) "_" STRINGIFY(PRODUCT_RATING_COLUMN_NAME), column_name)) {
        set_rating(sqlite3_column_double(statement, column_number));
    } else if(!strcmp(STRINGIFY(PRODUCT_TABLE_NAME) "_" STRINGIFY(PRODUCT_RATING_COUNT_COLUMN_NAME), column_name)) {
        set_rating_count(sqlite3_column_int(statement, column_number));
    } else {
        if(!strcmp(STRINGIFY(PRODUCT_TABLE_NAME), column_name)) {
            neroshop::print(std::string("Unexpected column name '")+column_name+std::string("' encountered while creating Product object."), 2);
        }
    }
}

std::string neroshop::Product::to_string() const {
    std::string str = "(Product> ";
    str += std::string("Id:")+get_id();
    str += std::string(", Name:")+get_name();
    str += std::string(", Description:")+get_description();
    str += std::string(", Weight:")+std::to_string(get_weight());
    str += std::string(", Attributes:")+get_attributes();
    str += std::string(", Code:")+get_code();
    str += std::string(", Category:")+std::to_string(get_category_id());
    str += std::string(", Rating:")+std::to_string(get_rating());
    str += std::string(", RatingCount:")+std::to_string(get_rating_count());
    str += ")";
    return str;
}

void neroshop::Product::set_id(std::string new_id) {
    id = new_id;
}

void neroshop::Product::set_id(const unsigned char* new_id) {
    std::string new_value = "NULL";
    if(new_id) {
        new_value = reinterpret_cast<const char*>(new_id);
    }
    set_id(new_value);
}

void neroshop::Product::set_name(std::string new_name) {
    name = new_name;
}

void neroshop::Product::set_name(const unsigned char* new_name) {
    std::string new_value = "NULL";
    if(new_name) {
        new_value = reinterpret_cast<const char*>(new_name);
    }
    set_name(new_value);
}

void neroshop::Product::set_description(std::string new_description) {
    description = new_description;
}

void neroshop::Product::set_description(const unsigned char* new_description) {
    std::string new_value = "NULL";
    if(new_description) {
        new_value = reinterpret_cast<const char*>(new_description);
    }
    set_description(new_value);
}

void neroshop::Product::set_weight(double new_weight) {
    weight = new_weight;
}

void neroshop::Product::set_attributes(std::string new_attributes) {
    attributes = new_attributes;
}

void neroshop::Product::set_attributes(const unsigned char* new_attributes) {
    std::string new_value = "NULL";
    if(new_attributes) {
        new_value = reinterpret_cast<const char*>(new_attributes);
    }
    set_attributes(new_value);
}

void neroshop::Product::set_code(std::string new_code) {
    code = new_code;
}

void neroshop::Product::set_code(const unsigned char* new_code) {
    std::string new_value = "NULL";
    if(new_code) {
        new_value = reinterpret_cast<const char*>(new_code);
    }
    set_code(new_value);
}

void neroshop::Product::set_category_id(int new_category_id) {
    category_id = new_category_id;
}

void neroshop::Product::set_rating(double new_rating) {
    rating = new_rating;
}

void neroshop::Product::set_rating_count(int new_rating_count) {
    rating_count = new_rating_count;
}

std::string neroshop::Product::get_id() const {
    return id;
}

std::string neroshop::Product::get_name() const {
    return name;
}

std::string neroshop::Product::get_description() const {
    return description;
}

double neroshop::Product::get_weight() const {
    return weight;
}

std::string neroshop::Product::get_attributes() const {
    return attributes;
}

std::string neroshop::Product::get_code() const {
    return code;
}

int neroshop::Product::get_category_id() const {
    return category_id;
}

double neroshop::Product::get_rating() const {
    return rating;
}

int neroshop::Product::get_rating_count() const {
    return rating_count;
}

#if defined(NEROSHOP_BUILD_GUI)
QVariantMap neroshop::Product::as_q_map() {
    QVariantMap out;
    out.insert("product_id", QString::fromStdString(get_id()));
    out.insert("product_uuid", QString::fromStdString(get_id()));
    out.insert("product_name", QString::fromStdString(get_name()));
    out.insert("product_description", QString::fromStdString(get_description()));
    out.insert("weight", get_weight());
    out.insert("product_attributes", QString::fromStdString(get_attributes()));
    out.insert("product_code", QString::fromStdString(get_code()));
    out.insert("product_category_id", get_category_id());
    out.insert("product_rating", get_rating());
    out.insert("product_rating_count", get_rating_count());
    return out;
}
#endif

std::string neroshop::operator+(std::string str, const neroshop::Product& product) {
    return str+product.to_string();
}

std::string neroshop::operator+(const neroshop::Product& product, std::string str) {
    return product.to_string()+str;
}

FILL_COLUMN_NAMES(neroshop::Product, PRODUCT_COLUMN_NAME_ARRAY)
