#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <tuple>

namespace neroshop {

struct Attribute {
    std::string color;
    std::string size; // ex. small, medium, large, etc.
    double weight;
    std::string material;
    std::tuple<double, double, double, std::string> dimensions; // the string can be in any of the following formats: "lxwxh" / "lwh", or "wxdxh" / "wdh", or "diameter x height" / "dh"
    std::string brand;
    std::string model; // can be a model type or a model number
    std::string manufacturer;
    std::string country_of_origin;
    std::string warranty_information;
    std::string product_code; // UPC, SKU, etc. // Product codes can be used to differentiate between variations of a product, such as different colors or bundle packages. So even if two Nintendo Switch Lite consoles are exactly the same, they may have different product codes if they are being sold as different variations (e.g. one is the gray version and the other is the yellow version).
    std::string style;
    std::string gender;
    std::pair<int, int> age_range;
    std::string energy_efficiency_rating; // A++, A+, A, B, C, ... G etc.
    std::vector<std::string> safety_features;
    unsigned int quantity_per_package;
    std::string release_date;
};

class Product { // can also be used for Services
public:
    Product(const std::string& id, const std::string& name, const std::string& description, const std::vector<Attribute>& attributes, const std::string& code, unsigned int category_id, const std::vector<std::string>& tags);
    
    void add_attribute(const Attribute& attribute);
    void add_variant(const Attribute& variant);
    void add_tag(const std::string& tag);
    
    void set_id(const std::string& id);
    void set_name(const std::string& name);
    void set_description(const std::string& description);
    void set_color(const std::string& color, int index = 0);
    void set_size(const std::string& size, int index = 0);
    void set_weight(double weight, int index = 0);
    void set_attributes(const std::vector<Attribute>& attributes);
    void set_variants(const std::vector<Attribute>& variants);
    void set_code(const std::string& code);
    void set_category_id(unsigned int id);
    void set_tags(const std::vector<std::string>& tags);

    std::string get_id() const;
    std::string get_name() const;
    std::string get_description() const;
    std::string get_color(int index = 0) const;
    std::string get_size(int index = 0) const;
    double get_weight(int index = 0) const;
    std::vector<Attribute> get_attributes() const;
    std::vector<Attribute> get_variants() const;
    std::string get_code() const;
    int get_category_id() const;//std::string get_category_as_string() const;
    std::vector<std::string> get_tags() const;
private:
    std::string id;
    std::string name;
    std::string description;
    std::vector<Attribute> attributes;
    std::string code; // optional - main product code
    unsigned int category_id;
    std::vector<std::string> tags; // optional
};

}
