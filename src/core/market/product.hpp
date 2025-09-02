#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <map>
#include <unordered_map>
#include <optional>

#include "image.hpp"

namespace neroshop {

struct ProductVariantInfo {
    std::string condition;             // overrides product condition if set
    std::optional<double> weight;      // overrides product weight if set
    std::optional<double> price;       // overrides base price if set
    std::optional<int> quantity;       // stock for this variant
    std::string product_code;          // variant-specific product code (e.g. SKU)
    std::vector<Image> images;         // variant-specific images
    std::optional<int> image_index;    // if using parent product images
};

struct ProductVariant {
    std::unordered_map<std::string, std::string> options; // e.g. {"color":"red","size":"L"}
    ProductVariantInfo info;
};

class Product { // can also be used for Services
public:
    Product();
    Product(const std::string& id, const std::string& name, const std::string& description, double weight, const std::vector<ProductVariant>& variants, const std::string& code, unsigned int category_id, const std::set<int>& subcategory_ids, const std::set<std::string>& tags, const std::vector<Image>& images);
    Product(const Product& other);// copy constructor
    Product(Product&& other) noexcept; // move constructor
    
    Product& operator=(const Product&); // copy assignment operator
    Product& operator=(Product&&) noexcept; // move assignment operator
    
    void add_variant(const ProductVariant& variant);
    void add_tag(const std::string& tag);
    void add_image(const Image& image);
    void print_product();
    std::vector<ProductVariant> cartesian_product() const; // Generates the Cartesian product of all variant option values, i.e., all possible variant combinations
    
    void set_id(const std::string& id);
    void set_name(const std::string& name);
    void set_description(const std::string& description);
    void set_weight(double weight);
    void set_variants(const std::vector<ProductVariant>& variants);
    void set_code(const std::string& code);
    void set_category(const std::string& category);
    void set_category_id(int category_id);
    void set_subcategories(const std::vector<std::string>& subcategories);
    void set_subcategory_ids(const std::set<int>& subcategory_ids);
    void set_tags(const std::set<std::string>& tags);

    std::string get_id() const;
    std::string get_name() const;
    std::string get_description() const;
    double get_weight() const;
    const std::vector<ProductVariant>& get_variants() const;
    std::map<std::string, std::set<std::string>> get_options() const; // Returns all the distinct option values per option name (e.g. { {"color", {"red", "green", "blue"}}, {"size", {"S", "M", "L"}} } => {"color": ["red", "green", "blue"], "size": ["S", "M", "L"]})
    std::string get_code() const;
    int get_category_id() const;
    std::string get_category_as_string() const;
    std::set<int> get_subcategory_ids() const;
    std::set<std::string> get_subcategories_as_string() const;
    std::set<std::string> get_tags() const;
    Image get_image(int index) const;
    std::vector<Image> get_images() const;
    
    bool variant_exists(const ProductVariant& variant) const;
private:
    void validate_variants() const;
    std::string id;
    std::string name;
    std::string description; // optional
    double weight; // optional - parent weight
    std::vector<ProductVariant> variants;
    std::string code; // optional - main product code
    unsigned int category_id;
    std::set<int> subcategory_ids; // optional
    std::set<std::string> tags; // optional
    std::vector<Image> images;
    static unsigned int max_options_per_variant; // options per variant - each product variant can have up to 2 options (e.g color, size, material, etc.)
    static unsigned int max_option_values; // option values per option - each option can have up to 10 values
    static unsigned int max_total_variants; // total variants = 10^2 = 100
};

}
