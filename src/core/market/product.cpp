#include "product.hpp"

#include "category.hpp"

#include <cstdint>    // uint64_t
#include <functional> // std::function
#include <limits>     // std::numeric_limits
#include <cmath>      // std::pow

namespace neroshop {

//-----------------------------------------------------------------------------

Product::Product() : id(""), name(""), description(""), weight(0.0), code(""), category_id(0), subcategory_ids({}) {}

//-----------------------------------------------------------------------------

Product::Product(const std::string& id, const std::string& name, const std::string& description, double weight, const std::vector<ProductVariant>& variants, const std::string& code, unsigned int category_id, const std::set<int>& subcategory_ids, const std::set<std::string>& tags, const std::vector<Image>& images)
    : id(id), name(name), description(description), weight(weight), variants(variants), code(code), category_id(category_id), subcategory_ids(subcategory_ids), tags(tags), images(images)
{}

//-----------------------------------------------------------------------------

Product::Product(const Product& other)
    : id(other.id), name(other.name), description(other.description), weight(other.weight), 
      variants(other.variants), code(other.code), category_id(other.category_id), 
      subcategory_ids(other.subcategory_ids), tags(other.tags), images(other.images)
{}

//-----------------------------------------------------------------------------

Product::Product(Product&& other) noexcept
    : id(std::move(other.id)),
      name(std::move(other.name)),
      description(std::move(other.description)),
      weight(std::move(other.weight)),
      variants(std::move(other.variants)),
      code(std::move(other.code)),
      category_id(std::move(other.category_id)),
      subcategory_ids(std::move(other.subcategory_ids)),
      tags(std::move(other.tags)),
      images(std::move(other.images))
{}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

unsigned int Product::max_options_per_variant (2);

//-----------------------------------------------------------------------------

unsigned int Product::max_option_values (10);

//-----------------------------------------------------------------------------

unsigned int Product::max_total_variants = [] { return static_cast<unsigned int>(std::pow(max_option_values, max_options_per_variant)); }();

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

neroshop::Product& Product::operator=(const neroshop::Product& other) {
    if (this != &other) {
        id = other.id;
        name = other.name;
        description = other.description;
        weight = other.weight;
        variants = other.variants;
        code = other.code;
        category_id = other.category_id;
        subcategory_ids = other.subcategory_ids;
        tags = other.tags;
        images = other.images;
    }
    return *this;
}

//-----------------------------------------------------------------------------

neroshop::Product& Product::operator=(neroshop::Product&& other) noexcept {
    if (this != &other) {
        id = std::move(other.id);
        name = std::move(other.name);
        description = std::move(other.description);
        weight = std::move(other.weight);
        variants = std::move(other.variants);
        code = std::move(other.code);
        category_id = std::move(other.category_id);
        subcategory_ids = std::move(other.subcategory_ids);
        tags = std::move(other.tags);
        images = std::move(other.images);

        // Set other object's fields to default values
        other.id = "";
        other.name = "";
        other.description = "";
        other.weight = 0.0;
        other.variants = {};
        other.code = "";
        other.category_id = 0;
        other.subcategory_ids = {};
        other.tags = {};
        other.images = {};
    }
    return *this;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void Product::add_variant(const ProductVariant& variant) {
    if (variant.options.size() > max_options_per_variant) {
        throw std::runtime_error("Too many options in variant, max allowed is " + std::to_string(max_options_per_variant));
    }
    variants.push_back(variant);
    validate_variants();
}

// Supports both single-option products (e.g. just “color”) and multi-option combinations

//-----------------------------------------------------------------------------

void Product::add_tag(const std::string& tag) {
    tags.insert(tag);
}

//-----------------------------------------------------------------------------

void Product::add_image(const Image& image) {
    images.push_back(image);
}

//-----------------------------------------------------------------------------

void Product::print_product() {
    std::cout << "Product ID: " << get_id() << std::endl;
    std::cout << "Name: " << get_name() << std::endl;
    std::cout << "Description: " << get_description() << std::endl;

    std::cout << "Variants: " << std::endl;
    for (const auto& variant : variants) {
        std::cout << " - ";
        for (const auto& [option_name, option_value] : variant.options) {
            std::cout << option_name << ": " << option_value << ", ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "Main Product Code: " << get_code() << std::endl;
    std::cout << "Category: " << get_category_as_string() << std::endl;//std::cout << "Category ID: " << get_category_id() << std::endl;
    for (int subcategory_id : subcategory_ids) {
        if (subcategory_id != -1) {
            std::cout << "Subcategory ID: " << subcategory_id << std::endl;
        }
    }
    std::cout << "Tags: ";
    for (const auto& tags : get_tags()) {
        std::cout << tags << ", ";
    }
    std::cout << std::endl;
}

//-----------------------------------------------------------------------------

std::vector<ProductVariant> Product::cartesian_product() const {
    auto variant_options = get_options();
    std::vector<ProductVariant> all_combinations;

    // Gather keys in deterministic order
    std::vector<std::string> keys;
    for (const auto& [opt_name, opt_value] : variant_options) {
        keys.push_back(opt_name);
    }

    // Recursive builder for all combinations
    std::function<void(size_t, ProductVariant)> build = [&](size_t idx, ProductVariant current) {
        if (idx == keys.size()) {
            all_combinations.push_back(std::move(current));
            return;
        }
        const auto& key = keys[idx];
        for (const auto& val : variant_options[key]) {
            ProductVariant next = current;
            next.options[key] = val;
            build(idx + 1, std::move(next));
        }
    };

    if (!keys.empty()) {
        build(0, ProductVariant{});
    }

    return all_combinations;
}

//-----------------------------------------------------------------------------

void Product::validate_variants() const {
    auto variant_options = get_options();
    if(variant_options.empty()) return;

    if (variant_options.size() > max_options_per_variant) {
        throw std::runtime_error("Exceeded max options per variant: " + std::to_string(max_options_per_variant));
    }

    uint64_t total_combinations = 1;
    for (const auto& [option_name, values] : variant_options) {
        if (values.size() > max_option_values) {
            throw std::runtime_error("Option '" + option_name + "' has too many values, max allowed is " + std::to_string(max_option_values));
        }
        // Check multiplication overflow
        if (total_combinations > std::numeric_limits<uint64_t>::max() / values.size()) {
            throw std::runtime_error("Multiplying option values size leads to integer overflow");
        }
        total_combinations *= static_cast<uint64_t>(values.size());

        if (total_combinations > max_total_variants) {
            throw std::runtime_error("Exceeded max total variant combinations: " + std::to_string(max_total_variants));
        }
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void Product::set_id(const std::string& id) {
    this->id = id;
}

//-----------------------------------------------------------------------------

void Product::set_name(const std::string& name) {
    this->name = name;
}

//-----------------------------------------------------------------------------

void Product::set_description(const std::string& description) {
    this->description = description;
}

//-----------------------------------------------------------------------------

void Product::set_weight(double weight) {
    this->weight = weight;
}

//-----------------------------------------------------------------------------

void Product::set_variants(const std::vector<ProductVariant>& variants) {
    for (const auto& variant : variants) {
        if (variant.options.size() > max_options_per_variant) {
            throw std::runtime_error("One or more variants have too many options (max allowed is " + std::to_string(max_options_per_variant) + ")");
        }
    }
    this->variants = variants;
    validate_variants();
}

//-----------------------------------------------------------------------------

void Product::set_code(const std::string& code) {
    this->code = code;
}

//-----------------------------------------------------------------------------

void Product::set_category(const std::string& category) {
    int category_id = get_category_id_by_name(category);
    set_category_id(category_id);
}

//-----------------------------------------------------------------------------

static bool is_valid_category_id(int id) {
    for (const auto& category : predefined_categories) {
        if (category.id == id) return true;
    }
    return false;
}

void Product::set_category_id(int category_id) {
    if(category_id == -1) { throw std::runtime_error("invalid category id"); }
    if (!is_valid_category_id(category_id)) {
        throw std::runtime_error("category id not found");
    }
    this->category_id = category_id;
}

//-----------------------------------------------------------------------------

void Product::set_subcategories(const std::vector<std::string>& subcategories) {
    std::set<int> subcategory_ids_set {};
    for (const std::string& subcategory : subcategories) {
        int category_id = get_category_id_by_name(subcategory);
        if (category_id != -1) {
            // If the name is a category, treat it as a valid subcategory
            subcategory_ids_set.insert(category_id);
            continue;
        }
    
        int subcategory_id = get_subcategory_id_by_name(subcategory);
        if(subcategory_id == -1) { 
            std::cerr << "Warning: invalid subcategory id for '" << subcategory << "'\n"; 
            continue; // Skip this subcategory
        }
        subcategory_ids_set.insert(subcategory_id);
    }
    set_subcategory_ids(subcategory_ids_set);
}

//-----------------------------------------------------------------------------

// TODO: check for any invalid subcategory_ids
void Product::set_subcategory_ids(const std::set<int>& subcategory_ids) {
    this->subcategory_ids = subcategory_ids;
}

//-----------------------------------------------------------------------------

void Product::set_tags(const std::set<std::string>& tags) {
    this->tags = tags;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

std::string Product::get_id() const {
    return id;
}

//-----------------------------------------------------------------------------

std::string Product::get_name() const {
    return name;
}

//-----------------------------------------------------------------------------

std::string Product::get_description() const {
    return description;
}

//-----------------------------------------------------------------------------

double Product::get_weight() const {
    return weight;
}

//-----------------------------------------------------------------------------

const std::vector<ProductVariant>& Product::get_variants() const {
    return variants;
}

//-----------------------------------------------------------------------------

std::map<std::string, std::set<std::string>> Product::get_options() const {
    std::map<std::string, std::set<std::string>> variant_options;

    for (const auto& variant : variants) {
        for (const auto& [option_name, option_value] : variant.options) {
            variant_options[option_name].insert(option_value);
        }
    }

    return variant_options;
}

//-----------------------------------------------------------------------------

std::string Product::get_code() const {
    return code;
}

//-----------------------------------------------------------------------------

int Product::get_category_id() const {
    return category_id;
}

//-----------------------------------------------------------------------------

std::string Product::get_category_as_string() const {
    return get_category_name_by_id(category_id);
}

//-----------------------------------------------------------------------------

std::set<int> Product::get_subcategory_ids() const {
    return subcategory_ids;
}

//-----------------------------------------------------------------------------

std::set<std::string> Product::get_subcategories_as_string() const {
    // Subcategories with unique ids may have duplicate names so we should use std::set instead
    std::set<std::string> subcategory_str_set {};
    for (int subcategory_id : this->subcategory_ids) {
        std::string subcategory = get_subcategory_name_by_id(subcategory_id);
        if (subcategory.empty()) {
            continue;
        }
        subcategory_str_set.insert(subcategory);
    }
    return subcategory_str_set;
}

//-----------------------------------------------------------------------------

std::set<std::string> Product::get_tags() const {
    return tags;
}

//-----------------------------------------------------------------------------

neroshop::Image Product::get_image(int index) const {
    if (index < 0 || index >= images.size()) throw std::out_of_range("get_image error: invalid index");
    return images[index];
}

//-----------------------------------------------------------------------------

std::vector<neroshop::Image> Product::get_images() const {
    return images;
}

//-----------------------------------------------------------------------------

}

