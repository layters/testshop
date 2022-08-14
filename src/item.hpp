#pragma once

#ifndef ITEM_HPP_NEROSHOP
#define ITEM_HPP_NEROSHOP

//#include <image.hpp>
#include <filesystem>
#include <iostream>
#include <vector>
#include <tuple> // std::tuple, std::get, std::tie, std::ignore

#include "database.hpp"

namespace neroshop {
class Item { // or Product or whatever you want to call it
public:
    Item();
    Item(unsigned int id); // copy an already registered item 
    Item(const std::string& name, const std::string& desc, 
        double price, double weight, double length, double width, double height,
        const std::string& condition, const std::string& product_code); // quantity is set by cart; discount is 0 by default
    Item(const std::string& name, const std::string& desc, double price, double weight, 
        const std::tuple<double, double, double>& size, 
        const std::string& condition, const std::string& product_code); // quantity is set by cart; discount is 0 by default      
    ~Item();
    void show_info();
    void upload(const std::string& filename);
    void delete_upload_image(int index);
    void delete_all_upload_images();
    //Image * get_upload_image(int index) const;
    int get_image_count() const;
    // getters
 	unsigned int get_quantity(unsigned int cart_id) const;
	static unsigned int get_quantity(unsigned int item_id, unsigned int cart_id);
    unsigned int get_id() const; // product id
    std::string get_name() const;
    static std::string get_name(unsigned int item_id);
    std::string get_description() const;
    static std::string get_description(unsigned int item_id);   
    double get_price() const; // original/list price - $500 for PS5 for example
    static double get_price(unsigned int item_id);
	double get_weight() const;
	static double get_weight(unsigned int item_id);   
	std::tuple<double, double, double> get_size() const;
	static std::tuple<double, double, double> get_size(unsigned int item_id);	 
	double get_discount(unsigned int seller_id) const;
	static double get_discount(unsigned int item_id, unsigned int seller_id);        
	std::string get_condition() const; // condition: new, used, used - good, used - like new, renewed
    static std::string get_condition(unsigned int item_id);
    std::string get_product_code() const;
    static std::string get_product_code(unsigned int item_id);    
    // seller functions    
    double get_seller_price(unsigned int seller_id) const;
    static double get_seller_price(unsigned int item_id, unsigned int seller_id); // seller/sales price - a scalper's $1000 for a PS5, for example     
	double get_seller_discount(unsigned int seller_id) const;
	static double get_seller_discount(unsigned int item_id, unsigned int seller_id);    
	std::string get_seller_condition(unsigned int seller_id) const; // condition: new, used, used - good, used - like new, renewed
    static std::string get_seller_condition(unsigned int item_id, unsigned int seller_id);     
    unsigned int get_stock_quantity() const;
    static unsigned int get_stock_quantity(unsigned int item_id);
    // added 2022-04-19
    unsigned int get_seller_id() const; // returns the id of a seller that has listed this item
    static unsigned int get_seller_id(unsigned int item_id);
    std::vector<unsigned int> get_seller_ids() const; // returns the ids of sellers that have listed this item
    static std::vector<unsigned int> get_seller_ids(unsigned int item_id);
    unsigned int get_seller_count() const; // returns the number of sellers that have listed this item
    static unsigned int get_seller_count(unsigned int item_id);
    // ratings / reviews - added 2022-02-15
    int get_ratings_count() const; // or get_reviews_count(); // returns total ratings   
    static int get_ratings_count(unsigned int item_id);
    int get_star_count(int star_number); // returns number of n stars
    static int get_star_count(unsigned int item_id, int star_number);
    float get_average_stars() const; // returns average stars
    static float get_average_stars(unsigned int item_id);
    ////////////////////////////////////
    // undefined getter functions
    std::string get_category() const;
    //static std::string get_category(unsigned int item_id);
    std::string get_subcategory() const;
    //static std::string get_subcategory(unsigned int item_id);
    //static double get_price(unsigned int item_id, unsigned int seller_id);
    //static _ get_(unsigned int item_id);
    double get_shipping_price() const;
    //static double get_shipping_price(unsigned int item_id);
    double get_total_price() const; // seller_price + shipping_price (no taxes except tx fees)
	//static double get_total_price(unsigned int item_id);  
    // get by_product_code - all product codes must be unique
    // get_item_id_by_name() const;
    // get_item_id_by_() const;
    // static ? get_item_id_by_product_code(const std::string& product_code);
    // static ? get_item_name_by_product_code(const std::string& product_code);
    // static ? get_item_description_by_product_code(const std::string& product_code);
    // static ? get_item_price_by_product_code(const std::string& product_code);
    // static ? get_item__by_product_code(const std::string& product_code);
    // get by_id - all item ids must be unique
    // static ? get_item_name_by_id(unsigned int id) const;
    // static ? get_item__by_id(unsigned int id);
    // static ? get_item__by_id(unsigned int id);
    // model_number, sku_number
    // boolean
    bool is_registered() const; // is registered in the database
    static bool is_registered(unsigned int item_id);
    bool is_saved() const; // saved to wishlist
    bool in_cart(/*unsigned int cart_id*/) const; // added to cart
    static bool in_cart(unsigned int item_id/*, unsigned int cart_id*/);
	bool in_stock() const; // is in stock - if false, then either unavailable or sold out
	static bool in_stock(unsigned int item_id);
	bool is_available() const; // is item available (if item inventory qty is 10, and the demand is high, then it will be "sold out")
	// friends
	friend class Seller; // seller can access item's private members
	friend class Cart; // cart can now access item's private members
	friend class Inventory;
	friend class Order;
private:
    static bool load_item(const Item& item, const std::string& item_name);  // load items database
    static void register_item(const Item& item); //private so can only be accessed by friend class Seller // will auto generate an id // store in inventory class
    void register_item(const std::string& name, const std::string& desc, 
        double price, double weight, double length, double width, double height,
        const std::string& condition, const std::string& product_code);
    void deregister_item();
    //static void deregister_item();
    unsigned int id; // unique id that must be obtained from the database
    std::string category;
    std::string SKU_code; // sellers can generate an SKU for items they are selling (NSKU - neroshop stocking unit)
    std::string UPC_code; // universal product code - ONLY code required for all products
    ////std::vector<std::shared_ptr<Image>> image_list;
    void set_id(unsigned int id);
    //void set_seller_price(unsigned int seller_id, double seller_price);
    //static void set_seller_price(unsigned int item_id, unsigned int seller_id, double seller_price); // seller_price is set by the seller
    // (all setters should be private)
    void set_quantity(unsigned int quantity, unsigned int cart_id);
    static void set_quantity(unsigned int item_id, unsigned int quantity, unsigned int cart_id); // item qty is managed by cart
    void set_name(const std::string& name);
    static void set_name(unsigned int item_id, const std::string& name);      
    void set_description(const std::string& description);
    static void set_description(unsigned int item_id, const std::string& description);    
    void set_price(double price);
    static void set_price(unsigned int item_id, double price);  
    void set_weight(double weight); // in kg or lbs.
    static void set_weight(unsigned int item_id, double weight);    
    void set_size(double l, double w, double h); // dimensions: length, width, and height (l x w x h)
    static void set_size(unsigned int item_id, double l, double w, double h);   
    void set_size(const std::tuple<double, double, double>& size);
    static void set_size(unsigned int item_id, const std::tuple<double, double, double>& size); 
    void set_discount(double discount);
    static void set_discount(unsigned int item_id, double discount);
    void set_discount_by_percentage(double percent); // item discount is managed by seller // convert discount (%) to decimal
    static void set_discount_by_percentage(unsigned int item_id, double percent);
    void set_condition(const std::string& condition); // seller sets the item condition    
    static void set_condition(unsigned int item_id, const std::string& condition);    
    void set_product_code(const std::string& product_code);
    static void set_product_code(unsigned int item_id, const std::string& product_code);        
    //void set_images(std::vector<char *>& images);
    //static void set_images(unsigned int item_id, std::vector<char *>& images);       
    //static void set_(unsigned int item_id, );
    static void create_categories_and_subcategories_table(void);
    static void create_item_table(void);
    static void create_table(void); // calls both create_categories_and_subcategories_table() and create_item_table()
};
}
#endif
