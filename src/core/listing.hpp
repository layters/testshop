#ifndef LISTING_HPP_NEROSHOP
#define LISTING_HPP_NEROSHOP

#include "product.hpp"
#include "listing_seller.hpp"

#if defined(NEROSHOP_BUILD_GUI)
#include <QJsonObject>
#endif

//Listing Table
#define LISTING_TABLE_NAME listings

#define LISTING_ID_COLUMN_NAME uuid
#define LISTING_PRODUCT_ID_COLUMN_NAME product_id
#define LISTING_SELLER_ID_COLUMN_NAME seller_id
#define LISTING_QUANTITY_COLUMN_NAME quantity
#define LISTING_PRICE_COLUMN_NAME price
#define LISTING_CURRENCY_COLUMN_NAME currency
#define LISTING_CONDITION_COLUMN_NAME condition
#define LISTING_LOCATION_COLUMN_NAME location
#define LISTING_DATE_COLUMN_NAME date
#define LISTING_COLUMN_NAME_ARRAY STRINGIFY(LISTING_ID_COLUMN_NAME), STRINGIFY(LISTING_SELLER_ID_COLUMN_NAME), STRINGIFY(LISTING_QUANTITY_COLUMN_NAME), STRINGIFY(LISTING_PRICE_COLUMN_NAME), STRINGIFY(LISTING_CURRENCY_COLUMN_NAME), STRINGIFY(LISTING_CONDITION_COLUMN_NAME), STRINGIFY(LISTING_LOCATION_COLUMN_NAME), STRINGIFY(LISTING_DATE_COLUMN_NAME)

namespace neroshop {
class Listing : public Row {
private:
    std::string id;
    Product product;
    ListingSeller seller;
    int quantity;
    double price;
    std::string currency;
    std::string condition;
    std::string location;
    std::string date;

protected:
    virtual void set_member_from_column_number(sqlite3_stmt* statement, const int column_number) override;

public:
    DECLARE_COLUMN_NAMES_MEMBERS(LISTING_TABLE_NAME)

    Listing();

    Listing(sqlite3_stmt* statement);

    std::string to_string() const;

    void set_id(std::string new_id);

    void set_product(Product new_product);

    void set_seller(ListingSeller new_seller);

    void set_quantity(int new_quantity);

    void set_price(double new_price);

    void set_currency(std::string new_currency);

    void set_currency(const unsigned char* new_currency);

    void set_condition(std::string new_condition);

    void set_condition(const unsigned char* new_condition);

    void set_location(std::string new_location);

    void set_location(const unsigned char* new_location);

    void set_date(std::string new_date);

    void set_date(const unsigned char* new_date);

    std::string get_id() const;

    Product get_product() const;

    ListingSeller get_seller() const;

    int get_quantity() const;

    double get_price() const;

    std::string get_currency() const;

    std::string get_condition() const;

    std::string get_location() const;

    std::string get_date() const;

#if defined(NEROSHOP_BUILD_GUI)
    QVariantMap as_q_map();
#endif
};

std::string operator+(std::string str, const Listing& listing);

std::string operator+(const Listing& listing, std::string str);
}
#endif
