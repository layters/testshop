// filename: catalog.hpp
//#pragma once // use #ifndef _HPP, #define _HPP, and #endif instead for portability

#ifndef CATALOG_HPP_NEROSHOP // recommended to add unique identifier like _NEROSHOP to avoid naming collision with other libraries
#define CATALOG_HPP_NEROSHOP
// neroshop
#include "debug.hpp"
#include "database.hpp"
#include "icon.hpp"
#include "item.hpp" // item price, details, and upload images
#include "converter.hpp" // currency conversion
#include "user.hpp" // for users' favorites/wishlist and verified purchases // cart is included here
// dokun-ui
#include <grid.hpp>
#include <spinner.hpp>
#include <button.hpp>
#include <toggle.hpp>
//#include <.hpp>
// STL
#include <iostream>
#include <memory> // std::unique_ptr, std::shared_ptr, std::make_shared

namespace neroshop {
class Catalog {
public:
    Catalog();//Catalog(int rows, int columns);
    ~Catalog();
    void draw();
    void draw(int x, int y);
    void center(int window_width, int window_height); // centers the current product page
    // setters
    //void set_columns(int columns); // this will be made private. 1 col = list_view | 2+ cols = grid_view
    void set_rows(int rows);
    void set_position(int x, int y);
    // for each individual boxes
    void set_box_width(int box_width);
    void set_box_height(int box_height);
    void set_box_size(int box_width, int box_height);
    void set_box_size(const Vector2i& box_size);
    // getters
    Grid * get_grid() const; // contains all the boxes
    Box * get_box(int row, int column) const;
    Box * get_box(int index) const;
    Box * get_box_by_item_id(unsigned int item_id) const;
    Grid * get_view() const; // same as get_grid
    Box * get_page() const; // returns current page
    Box * get_tooltip() const; // returns tooltip
    Box * get_sort_box() const; // returns sort_box
    int get_x() const;
    int get_y() const;
    Vector2i get_position() const;
    int get_width() const;
    int get_height() const;
    Vector2i get_size() const;
    //get_capacity(); // number of boxes that can be lined up horizontally
    //get_row_capacity(); // number of boxes that can be lined up vertically
private:
    std::unique_ptr<Box> current; // shows product_page (you can only view one product page at a time) // make this static
    std::unique_ptr<Grid> view; // shows catalog_page (item listings)
    std::unique_ptr<Box> tooltip;
    std::unique_ptr<Box> sort_box; // sort and filter by brand, color, size, type, price_range, customer reviews (ratings), etc.
    neroshop::Cart * cart; // the cart that is currently being served by the catalog // will copy the user's cart (we don't have to delete the cart since the User::cart's unique_ptr owns it)
    void initialize();
    void update(); // updates size and width of boxes
    //void on_draw();
public:
    // catalog view (grid) functions -----------------------------
    void populate(); // fills / populates category view with items in inventory
    //populate_by_category, populate_by_best_seller (check table order_item -> item_id)
    // populate_by_latest,  populate_by_best_deals_and_promo
    // we can show featured items, best sellers, 
    void refresh(const neroshop::User& user); // refresh the contents
    // fetching product information from the database ------------
    void fetch_items(); // fetches all items that have been registered
    void fetch_inventory(); // fetches all inventory items (regardless of whether they are in stock or not)
    void fetch_items_and_inventory(); // fetches joined tables item and inventory
    void fetch_best_sellers(); // fetches the best-selling items
    void fetch_most_favorited(); // fetches the most favorited items
    // catalog current product page (box) functions --------------
    void setup_page();
    void update_page(int item_id);
    // sorting/filtering box functions ---------------------------
    void setup_filters();
    // views: list_view (1 column, multiple rows), grid_view (multiple rows, multiple columns)
};
}
#endif
// https://github.com/kyliau/ShoppingCart/blob/master/Catalog.cpp
