#include <cstring>
#include "debug.hpp"
#include "listing_seller.hpp"

neroshop::ListingSeller::ListingSeller() : id(""), name(""), public_key("NULL"), avatar_blob(0) {
}

neroshop::ListingSeller::ListingSeller(sqlite3_stmt* statement) : neroshop::ListingSeller() {
    initialize_from_statement(statement);
}

void neroshop::ListingSeller::set_member_from_column_number(sqlite3_stmt* statement, const int column_number) {
    const char* column_name = sqlite3_column_name(statement, column_number);
    if(!strcmp(STRINGIFY(SELLER_TABLE_NAME) "_" STRINGIFY(SELLER_ID_COLUMN_NAME), column_name)) {
        set_id(reinterpret_cast<const char*>(sqlite3_column_text(statement, column_number)));
    } else if(!strcmp(STRINGIFY(SELLER_TABLE_NAME) "_" STRINGIFY(SELLER_NAME_COLUMN_NAME), column_name)) {
        set_name(sqlite3_column_text(statement, column_number));
    } else if(!strcmp(STRINGIFY(SELLER_TABLE_NAME) "_" STRINGIFY(SELLER_KEY_COLUMN_NAME), column_name)) {
        set_public_key(sqlite3_column_text(statement, column_number));
    } else if(!strcmp(STRINGIFY(SELLER_TABLE_NAME) "_" STRINGIFY(SELLER_AVATAR_COLUMN_NAME), column_name)) {
        set_avatar_blob(sqlite3_column_blob(statement, column_number), sqlite3_column_bytes(statement, column_number));
    } else {
        if(!strcmp(STRINGIFY(SELLER_TABLE_NAME), column_name)) {
            neroshop::print(std::string("Unexpected column name '")+column_name+std::string("' encountered while creating Seller object."), 2);
        }
    }
}

std::string neroshop::ListingSeller::to_string() const {
    std::string str = "(Seller> ";
    str += std::string("Id:")+get_id();
    str += std::string(", Name:")+get_name();
    str += std::string(", Public Key:")+get_public_key();
    str += std::string(", Avatar:")+get_avatar_blob_as_string();
    str += ")";
    return str;
}

void neroshop::ListingSeller::set_id(std::string new_id) {
    id = new_id;
}

void neroshop::ListingSeller::set_name(std::string new_name) {
    name = new_name;
}

void neroshop::ListingSeller::set_name(const unsigned char* new_name) {
    std::string new_value = "NULL";
    if(new_name) {
        new_value = reinterpret_cast<const char*>(new_name);
    }
    set_name(new_value);
}


void neroshop::ListingSeller::set_public_key(std::string new_public_key) {
    public_key = new_public_key;
}

void neroshop::ListingSeller::set_public_key(const unsigned char* new_public_key) {
    std::string new_value = "NULL";
    if(new_public_key) {
        new_value = reinterpret_cast<const char*>(new_public_key);
    }
    set_public_key(new_value);
}

void neroshop::ListingSeller::set_avatar_blob(const void* new_avatar_blob, int size) {
    if(new_avatar_blob) {
        avatar_blob.resize(size);
        for(int i = 0; i < size; ++i) {
            avatar_blob[i] = reinterpret_cast<const unsigned char*>(new_avatar_blob)[i];
        }
    }
}

std::string neroshop::ListingSeller::get_id() const {
    return id;
}

std::string neroshop::ListingSeller::get_name() const {
    return name;
}

std::string neroshop::ListingSeller::get_public_key() const {
    return public_key;
}

std::vector<unsigned char> neroshop::ListingSeller::get_avatar_blob() const {
    return avatar_blob;
}

std::string neroshop::ListingSeller::get_avatar_blob_as_string() const {
    std::string avatar_blob_as_string("");
    for(int i = 0; i < avatar_blob.size(); ++i) {
        avatar_blob_as_string += avatar_blob[i];
    }
    return avatar_blob_as_string;
}

std::string neroshop::operator+(std::string str, const neroshop::ListingSeller& seller) {
    return str+seller.to_string();
}

std::string neroshop::operator+(const neroshop::ListingSeller& seller, std::string str) {
    return seller.to_string()+str;
}

FILL_COLUMN_NAMES(neroshop::ListingSeller, SELLER_COLUMN_NAME_ARRAY)
