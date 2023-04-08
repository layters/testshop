#ifndef LISTING_SELLER_HPP_NEROSHOP
#define LISTING_SELLER_HPP_NEROSHOP

#include "row.hpp"

//Seller Table
#define SELLER_TABLE_NAME users

#define SELLER_ID_COLUMN_NAME monero_address
#define SELLER_NAME_COLUMN_NAME name
#define SELLER_KEY_COLUMN_NAME public_key
#define SELLER_AVATAR_COLUMN_NAME avatar
#define SELLER_COLUMN_NAME_ARRAY STRINGIFY(SELLER_ID_COLUMN_NAME), STRINGIFY(SELLER_NAME_COLUMN_NAME), STRINGIFY(SELLER_KEY_COLUMN_NAME), STRINGIFY(SELLER_AVATAR_COLUMN_NAME)

namespace neroshop {
class ListingSeller : Row {
private:
    std::string id;
    std::string name;
    std::string public_key;
    std::vector<unsigned char> avatar_blob;

protected:
    virtual void set_member_from_column_number(sqlite3_stmt* statement, const int column_number) override;

public:
    DECLARE_COLUMN_NAMES_MEMBERS(SELLER_TABLE_NAME)

    ListingSeller();

    ListingSeller(sqlite3_stmt* statement);

    std::string to_string() const;

    void set_id(std::string new_id);

    void set_name(std::string new_name);

    void set_name(const unsigned char* new_name);

    void set_public_key(std::string new_public_key);

    void set_public_key(const unsigned char* new_public_key);

    void set_avatar_blob(const void* new_avatar_blob, int size);

    std::string get_id() const;

    std::string get_name() const;

    std::string get_public_key() const;

    std::vector<unsigned char> get_avatar_blob() const;

    std::string get_avatar_blob_as_string() const;
};

std::string operator+(std::string str, const ListingSeller& seller);

std::string operator+(const ListingSeller& seller, std::string str);
}
#endif
