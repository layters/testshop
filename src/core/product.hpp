#ifndef PRODUCT_HPP_NEROSHOP
#define PRODUCT_HPP_NEROSHOP

#include "row.hpp"

#if defined(NEROSHOP_BUILD_GUI)
#include <QJsonObject>
#endif

//Product Table
#define PRODUCT_TABLE_NAME products

#define PRODUCT_ID_COLUMN_NAME uuid
#define PRODUCT_NAME_COLUMN_NAME name
#define PRODUCT_DESCRIPTION_COLUMN_NAME description
#define PRODUCT_WEIGHT_COLUMN_NAME weight
#define PRODUCT_ATTRIBUTES_COLUMN_NAME attributes
#define PRODUCT_CODE_COLUMN_NAME code
#define PRODUCT_CATEGORY_ID_COLUMN_NAME category_id
#define PRODUCT_COLUMN_NAME_ARRAY STRINGIFY(PRODUCT_ID_COLUMN_NAME), STRINGIFY(PRODUCT_NAME_COLUMN_NAME), STRINGIFY(PRODUCT_DESCRIPTION_COLUMN_NAME), STRINGIFY(PRODUCT_WEIGHT_COLUMN_NAME), STRINGIFY(PRODUCT_ATTRIBUTES_COLUMN_NAME), STRINGIFY(PRODUCT_CODE_COLUMN_NAME), STRINGIFY(PRODUCT_CATEGORY_ID_COLUMN_NAME)

//Product Ratings Table
#define PRODUCT_RATINGS_TABLE_NAME product_ratings
#define PRODUCT_RATINGS_PRODUCT_ID_COLUMN_NAME product_id
#define PRODUCT_RATINGS_STARS_COLUMN_NAME stars

//Generated Product Columns
#define PRODUCT_RATING_COLUMN_NAME rating
#define PRODUCT_RATING_COUNT_COLUMN_NAME rating_count

namespace neroshop {
class Product : public Row {
private:
    std::string id;
    std::string name;
    std::string description;
    double weight;
    std::string attributes;
    std::string code;
    int category_id;
    double rating;
    int rating_count;

protected:
    virtual void set_member_from_column_number(sqlite3_stmt* statement, const int column_number) override;

public:
    DECLARE_COLUMN_NAMES_MEMBERS(PRODUCT_TABLE_NAME)

    Product();

    Product(sqlite3_stmt* statement);

    std::string to_string() const;

    void set_id(std::string new_id);

    void set_id(const unsigned char* new_id);

    void set_name(std::string new_name);

    void set_name(const unsigned char* new_name);

    void set_description(std::string new_description);

    void set_description(const unsigned char* new_description);

    void set_weight(double new_weight);

    void set_attributes(std::string new_attributes);

    void set_attributes(const unsigned char* new_attributes);

    void set_code(std::string new_code);

    void set_code(const unsigned char* new_code);

    void set_category_id(int new_category_id);

    void set_rating(double new_rating);

    void set_rating_count(int new_rating_count);

    std::string get_id() const;

    std::string get_name() const;

    std::string get_description() const;

    double get_weight() const;

    std::string get_attributes() const;

    std::string get_code() const;

    int get_category_id() const;

    double get_rating() const;

    int get_rating_count() const;

#if defined(NEROSHOP_BUILD_GUI)
    QVariantMap as_q_map();
#endif
};

std::string operator+(std::string str, const Product& product);

std::string operator+(const Product& product, std::string str);
}
#endif
