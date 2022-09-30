#pragma once

#ifndef CATALOG_HPP_NEROSHOP
#define CATALOG_HPP_NEROSHOP

#include <iostream>
#include <memory> // std::unique_ptr, std::shared_ptr, std::make_shared

#include "debug.hpp"
#include "database.hpp"
#include "item.hpp" // item price, details, and upload images
#include "converter.hpp" // currency conversion
#include "user.hpp" // for users' favorites/wishlist and verified purchases // cart is included here

namespace neroshop {
class Catalog {
public:
    Catalog();//Catalog(int rows, int columns);
    ~Catalog();
    // setters
    // getters
private:
    neroshop::Cart * cart; // the cart that is currently being served by the catalog // will copy the user's cart (we don't have to delete the cart since the User::cart's unique_ptr owns it)
    void initialize();
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
