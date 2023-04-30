#include "product.hpp"

neroshop::Product::Product(const std::string& id, const std::string& name, const std::string& description, const std::vector<neroshop::Attribute>& attributes, const std::string& code, unsigned int category_id)
    : id(id), name(name), description(description), attributes(attributes), code(code), category_id(category_id)
{}


//-----------------------------------------------------------------------------

void neroshop::Product::add_attribute(const Attribute& attribute) {
    attributes.push_back(attribute);
}

void neroshop::Product::add_variant(const Attribute& variant) {
    add_attribute(variant);
}

//-----------------------------------------------------------------------------

void neroshop::Product::set_id(const std::string& id) {
    this->id = id;
}

void neroshop::Product::set_name(const std::string& name) {
    this->name = name;
}

void neroshop::Product::set_description(const std::string& description) {
    this->description = description;
}

void neroshop::Product::set_color(const std::string& color, int index) {
    if (index < 0 || index >= attributes.size()) throw std::out_of_range("set_color error: invalid index");
    attributes[index].color = color;
}

void neroshop::Product::set_size(const std::string& size, int index) {
    if (index < 0 || index >= attributes.size()) throw std::out_of_range("set_size error: invalid index");
    attributes[index].size = size;
}

void neroshop::Product::set_weight(double weight, int index) {
    if (index < 0 || index >= attributes.size()) throw std::out_of_range("set_weight error: invalid index");
    attributes[index].weight = weight;
}

void neroshop::Product::set_attributes(const std::vector<neroshop::Attribute>& attributes) {
    this->attributes = attributes;
}

void neroshop::Product::set_variants(const std::vector<neroshop::Attribute>& variants) {
    set_attributes(variants);
}

void neroshop::Product::set_code(const std::string& code) {
    this->code = code;
}

void neroshop::Product::set_category_id(unsigned int category_id) {
    this->category_id = category_id;
}

//-----------------------------------------------------------------------------

std::string neroshop::Product::get_id() const {
    return id;
}

std::string neroshop::Product::get_name() const {
    return name;
}

std::string neroshop::Product::get_description() const {
    return description;
}

std::string neroshop::Product::get_color(int index) const {
    if (index < 0 || index >= attributes.size()) throw std::out_of_range("get_color error: invalid index");
    return attributes[index].color;
}

std::string neroshop::Product::get_size(int index) const {
    if (index < 0 || index >= attributes.size()) throw std::out_of_range("get_size error: invalid index");
    return attributes[index].size;
}

double neroshop::Product::get_weight(int index) const {
    if (index < 0 || index >= attributes.size()) throw std::out_of_range("get_weight error: invalid index");
    return attributes[index].weight;
}

std::vector<neroshop::Attribute> neroshop::Product::get_attributes() const {
    return attributes;
}

std::vector<neroshop::Attribute> neroshop::Product::get_variants() const {
    return get_attributes();
}

std::string neroshop::Product::get_code() const {
    return code;
}

int neroshop::Product::get_category_id() const {
    return category_id;
}

//std::string get_category_as_string() const {}

