#include "product.hpp"

#include "category.hpp"

neroshop::Product::Product() : id(""), name(""), description(""), code(""), category_id(0)/*, subcategory_id(-1)*/ {}

neroshop::Product::Product(const std::string& id, const std::string& name, const std::string& description, const std::vector<neroshop::Attribute>& attributes, const std::string& code, unsigned int category_id, const std::vector<int>& subcategory_ids, const std::vector<std::string>& tags, const std::vector<Image>& images)
    : id(id), name(name), description(description), attributes(attributes), code(code), category_id(category_id), subcategory_ids(subcategory_ids), tags(tags), images(images)
{}

neroshop::Product::Product(const Product& other)
    : id(other.id), name(other.name), description(other.description), 
      attributes(other.attributes), code(other.code), category_id(other.category_id), 
      subcategory_ids(other.subcategory_ids), tags(other.tags), images(other.images)
{}

neroshop::Product::Product(Product&& other) noexcept
    : id(std::move(other.id)),
      name(std::move(other.name)),
      description(std::move(other.description)),
      attributes(std::move(other.attributes)),
      code(std::move(other.code)),
      category_id(std::move(other.category_id)),
      subcategory_ids(std::move(other.subcategory_ids)),
      tags(std::move(other.tags)),
      images(std::move(other.images))
{}

//-----------------------------------------------------------------------------

neroshop::Product& neroshop::Product::operator=(const neroshop::Product& other) {
    if (this != &other) {
        id = other.id;
        name = other.name;
        description = other.description;
        attributes = other.attributes;
        code = other.code;
        category_id = other.category_id;
        subcategory_ids = other.subcategory_ids;
        tags = other.tags;
        images = other.images;
    }
    return *this;
}

neroshop::Product& neroshop::Product::operator=(neroshop::Product&& other) noexcept {
    if (this != &other) {
        id = std::move(other.id);
        name = std::move(other.name);
        description = std::move(other.description);
        attributes = std::move(other.attributes);
        code = std::move(other.code);
        category_id = std::move(other.category_id);
        subcategory_ids = std::move(other.subcategory_ids);
        tags = std::move(other.tags);
        images = std::move(other.images);

        // Set other object's fields to default values
        other.id = "";
        other.name = "";
        other.description = "";
        other.attributes = {};
        other.code = "";
        other.category_id = 0;
        other.subcategory_ids = {};
        other.tags = {};
        other.images = {};
    }
    return *this;
}

//-----------------------------------------------------------------------------

void neroshop::Product::add_attribute(const Attribute& attribute) {
    attributes.push_back(attribute);
}

void neroshop::Product::add_variant(const Attribute& variant) {
    add_attribute(variant);
}

void neroshop::Product::add_tag(const std::string& tag) {
    tags.push_back(tag);
}

void neroshop::Product::add_image(const Image& image) {
    images.push_back(image);
}

void neroshop::Product::print_product() {
    std::cout << "Product ID: " << get_id() << std::endl;
    std::cout << "Name: " << get_name() << std::endl;
    std::cout << "Description: " << get_description() << std::endl;

    std::vector<neroshop::Attribute> attributes = get_attributes();
    for (size_t i = 0; i < attributes.size(); i++) {
        if(i != 0) std::cout << std::endl;
        if(attributes.size() > 1) std::cout << "Variant #" << i+1 << std::endl;
        if(!attributes[i].color.empty()) std::cout << "Color: " << attributes[i].color << std::endl;
        if(!attributes[i].size.empty()) std::cout << "Size: " << attributes[i].size << std::endl;
        if(attributes[i].weight != 0.0) std::cout << "Weight: " << attributes[i].weight << std::endl;
        if(!attributes[i].material.empty()) std::cout << "Material: " << attributes[i].material << std::endl;
        if(!std::get<3>(attributes[i].dimensions).empty()) std::cout << "Dimensions: " << std::get<0>(attributes[i].dimensions) << "x" << std::get<1>(attributes[i].dimensions) << "x" << std::get<2>(attributes[i].dimensions) << " (" << std::get<3>(attributes[i].dimensions) << ")" << std::endl;
        if(!attributes[i].brand.empty()) std::cout << "Brand: " << attributes[i].brand << std::endl;
        if(!attributes[i].model.empty()) std::cout << "Model: " << attributes[i].model << std::endl;
        if(!attributes[i].manufacturer.empty()) std::cout << "Manufacturer: " << attributes[i].manufacturer << std::endl;
        if(!attributes[i].country_of_origin.empty()) std::cout << "Country of Origin: " << attributes[i].country_of_origin << std::endl;
        if(!attributes[i].warranty_information.empty()) std::cout << "Warranty Information: " << attributes[i].warranty_information << std::endl;
        if(!attributes[i].product_code.empty()) std::cout << "Product Code: " << attributes[i].product_code << std::endl;
        if(!attributes[i].style.empty()) std::cout << "Style: " << attributes[i].style << std::endl;
        if(!attributes[i].gender.empty()) std::cout << "Gender: " << attributes[i].gender << std::endl;
        if(attributes[i].age_range.second > 0) std::cout << "Age Range: " << attributes[i].age_range.first << "-" << attributes[i].age_range.second << std::endl;
        if(!attributes[i].energy_efficiency_rating.empty()) std::cout << "Energy Efficiency Rating: " << attributes[i].energy_efficiency_rating << std::endl;
        if(!attributes[i].safety_features.empty()) {
            std::cout << "Safety Features: ";
            for (const auto& feature : attributes[i].safety_features) {
                std::cout << feature << ", ";
            }
            std::cout << std::endl;
        }        
        if(attributes[i].quantity_per_package != 0) std::cout << "Quantity per Package: " << attributes[i].quantity_per_package << std::endl;
        if(!attributes[i].release_date.empty()) std::cout << "Release Date: " << attributes[i].release_date << std::endl;
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
    if(index == 0 && attributes.empty()) {
        attributes.emplace_back(Attribute{.color = color});
        return;
    }
    if (index < 0 || index >= attributes.size()) throw std::out_of_range("set_color error: invalid index");
    attributes[index].color = color;
}

void neroshop::Product::set_size(const std::string& size, int index) {
    if(index == 0 && attributes.empty()) {
        attributes.emplace_back(Attribute{.size = size});
        return;
    }
    if (index < 0 || index >= attributes.size()) throw std::out_of_range("set_size error: invalid index");
    attributes[index].size = size;
}

void neroshop::Product::set_weight(double weight, int index) {
    if(index == 0 && attributes.empty()) {
        attributes.emplace_back(Attribute{.weight = weight});
        return;
    }
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

void neroshop::Product::set_category(const std::string& category) {
    int category_id = get_category_id_by_name(category);
    set_category_id(category_id);
}

void neroshop::Product::set_category_id(unsigned int category_id) {
    this->category_id = category_id;
}

void neroshop::Product::set_subcategories(const std::vector<std::string>& subcategories) {
    std::vector<int> subcategory_id_vector {};
    for (const std::string& subcategory : subcategories) {
        int subcategory_id = get_category_id_by_name(subcategory);
        subcategory_id_vector.push_back(subcategory_id);
    }
    set_subcategory_ids(subcategory_id_vector);
}

void neroshop::Product::set_subcategory_ids(const std::vector<int>& subcategory_ids) {
    this->subcategory_ids = subcategory_ids;
}

void neroshop::Product::set_tags(const std::vector<std::string>& tags) {
    this->tags = tags;
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

std::string neroshop::Product::get_category_as_string() const {
    return get_category_name_by_id(category_id);
}

std::vector<int> neroshop::Product::get_subcategory_ids() const {
    return subcategory_ids;
}

std::vector<std::string> neroshop::Product::get_subcategories_as_string() const {
    std::vector<std::string> subcategory_str_vector {};
    for (int subcategory_id : this->subcategory_ids) {
        std::string subcategory = get_subcategory_name_by_id(subcategory_id);
        subcategory_str_vector.push_back(subcategory);
    }
    return subcategory_str_vector;
}

std::vector<std::string> neroshop::Product::get_tags() const {
    return tags;
}

neroshop::Image neroshop::Product::get_image(int index) const {
    if (index < 0 || index >= images.size()) throw std::out_of_range("get_image error: invalid index");
    return images[index];
}

std::vector<neroshop::Image> neroshop::Product::get_images() const {
    return images;
}

