#include "catalog.hpp"

#include "database.hpp"
#include "util/logger.hpp"
#include "item.hpp" // item price, details, and upload images
#include "currency_converter.hpp" // currency conversion
////////////////////
neroshop::Catalog::Catalog() : cart(nullptr) {
    initialize();
}
////////////////////
neroshop::Catalog::~Catalog() {
    std::cout << "catalog deleted\n";
}
////////////////////
////////////////////
void neroshop::Catalog::initialize() {
}
////////////////////
void neroshop::Catalog::setup_page() {
}
////////////////////
void neroshop::Catalog::update_page(int item_id) {
}
////////////////////
void neroshop::Catalog::setup_filters() {
}
////////////////////
////////////////////
////////////////////
// This function will change overtime and will populate the catalog with products such as best sellers, most wished for, etc.
void neroshop::Catalog::populate() {
    //fetch_items();
    fetch_inventory();
    //fetch_best_sellers();
    //fetch_most_favorited();
    //-----------------------------------
    neroshop::print("catalog populated");
}
////////////////////
void neroshop::Catalog::fetch_items() {
#if defined(NEROSHOP_USE_POSTGRESQL)
    std::string command = "SELECT * FROM item ORDER BY id ASC LIMIT $1;"; // "ORDER BY id ASC" = place in ascending order by id (oldest to latest items). DESC would be from the latest to the oldest items
    int box_count = view->get_row_count() * view->get_column_count(); // rows x columns
    std::vector<const char *> param_values = { std::to_string(box_count).c_str() };
    PGresult * result = PQexecParams(DB::Postgres::get_singleton()->get_handle(), command.c_str(), 1, nullptr, param_values.data(), nullptr, nullptr, 0);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        neroshop::print("Catalog::fetch(): No registered items found", 2);        
        PQclear(result);//DB::Postgres::get_singleton()->finish();//exit(1);
        return; // exit so we don't double free "result"
    }
    int rows = PQntuples(result); // max_rows is limited to number of boxes in the grid//std::cout << "number of rows (items): " << rows << std::endl;
    for(int i = 0; i < std::min<size_t>(rows, box_count); i++) {
        Box * box = view->get_box(i);//(r, c);
        int item_id = std::stoi(PQgetvalue(result, i, 0));
        std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_id) });
        std::cout << "registered item_ids: " << item_id << " (" << item_name << ")" << std::endl;        
        // draw contents here ...
    }
    ////////////////////
    PQclear(result); // free result when done using it
#endif    
}
////////////////////
void neroshop::Catalog::fetch_inventory() {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // here, we use DISTINCT ON to ignore duplicate items with the same item_id
    std::string command = "SELECT DISTINCT ON (item_id) * FROM inventory ORDER BY item_id ASC LIMIT $1;";//WHERE stock_qty > 0;";// "ORDER BY item_id ASC" = place in ascending order by item_id (lowest to highest). DESC would be from highest to lowest
    int box_count = view->get_row_count() * view->get_column_count(); // rows x columns
    std::vector<const char *> param_values = { std::to_string(box_count).c_str() };
    PGresult * result = PQexecParams(DB::Postgres::get_singleton()->get_handle(), command.c_str(), 1, nullptr, param_values.data(), nullptr, nullptr, 0);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        neroshop::print("Catalog::fetch(): No inventory items found", 2);        
        PQclear(result);//DB::Postgres::get_singleton()->finish();//exit(1);
        return; // exit so we don't double free "result"
    }
    int rows = PQntuples(result); // max_rows is limited to number of boxes in the grid//std::cout << "number of rows (items): " << rows << std::endl;
    for(int i = 0; i < std::min<size_t>(rows, box_count); i++) {
        /*std::cout << PQgetvalue(result, i, 0) << std::endl; // inventory_id
        std::cout << PQgetvalue(result, i, 1) << std::endl; // item_id
        std::cout << PQgetvalue(result, i, 2) << std::endl; // seller_id
        std::cout << PQgetvalue(result, i, 3) << std::endl; // stock_qty
        std::cout << PQgetvalue(result, i, 4) << std::endl; // seller_price or sales_price (not the same as list_price)
        std::cout << PQgetvalue(result, i, 5) << std::endl; // seller's currency of choice (which will be converted to buyer's native currency at checkout)
        std::cout << PQgetvalue(result, i, 6) << std::endl; // seller_discount
        std::cout << PQgetvalue(result, i, 7) << std::endl; // discount_qty
        std::cout << PQgetvalue(result, i, 8) << std::endl; // discount_times
        std::cout << PQgetvalue(result, i, 9) << std::endl; // discount_expiry
        std::cout << PQgetvalue(result, i, 10) << std::endl; // item_condition
        //std::cout << PQgetvalue(result, i, 11) << std::endl; //std::cout << PQgetvalue(result, i, 12) << std::endl;*/
        ///////////////////////////////////////////////////////////////////////////    
	////for(int r = 0; r < view->get_box_list_2d().size(); r++) // block.size() = rows
	////{
		////for(int c = 0; c < view->get_box_list_2d()[r].size(); c++) { // block[r] = items in row r	
            Box * box = view->get_box(i);//(r, c);
            int item_id = std::stoi(PQgetvalue(result, i, 1)); //std::cout << "item_ids: " << item_id << std::endl;
            // I guess we dont need to use smart pointers all the time, but when creating an object with "new", it should be immediately stored in a smart pointer so that it will be automatically deleted at the appropriate time: https://stackoverflow.com/questions/26473733/using-smart-pointers-as-a-class-member#comment41585405_26473733
            // verified_purchase_icon - maybe replace this with tickers (e.g "recommended", "new", "best seller"), etc. (verified purchase icon cannot be pressed but is only shown to let the user know whether they have previously purchased this item or not)
            Image * verified_purchase_icon = new Image(Icon::get["paid"]->get_data(), 64, 64, 1, 4); // image is still alive outside of scope :D
            //std::shared_ptr<Image> verified_purchase_icon = std::shared_ptr<Image>(new Image(Icon::get["paid"]->get_data(), 64, 64, 1, 4)); // causes "Floating point exception (core dumped)" error
            //std::shared_ptr<Image> verified_purchase_icon = std::make_shared<Image>(Icon::get["paid"]->get_data(), 64, 64, 1, 4); // image is dead at end of scope :(
            verified_purchase_icon->resize(24, 24);//(32, 32);
            verified_purchase_icon->set_color(128, 128, 128, 1.0);//(30, 80, 155); //155 or 255
            verified_purchase_icon->set_relative_position(10, 10);
            verified_purchase_icon->set_visible(false); // hide by default and show only if user has purchased the item (guests purchases are not stored which is another reason why verified_purchase_icon is hidden)
            box->add_image(*verified_purchase_icon);
            // heart_icon (favorites or wishlist)
            Image * heart_icon = new Image(Icon::get["heart"]->get_data(), 64, 64, 1, 4);
            heart_icon->set_color(128, 128, 128, 1.0);//(224, 93, 93, 1.0);
            heart_icon->resize(verified_purchase_icon->get_width(), verified_purchase_icon->get_height());//(24, 24);//(16, 16);
            heart_icon->set_relative_position(box->get_width() - heart_icon->get_width() - 10, 10);
            heart_icon->set_visible(false); // hide by default and show only if user has favorited the item (guests cannot heart items which is another reason why heart_icon is hidden)
            box->add_image(*heart_icon);
            // product_image (thumbnail)     
            Item item(item_id); // temporary (will die at end of scope)
            Image * product_image = item.get_upload_image(1); // first image is thumbnail
            if(!product_image) product_image = new Image(Icon::get["image_gallery"]->get_data(), 64, 64, 1, 4); // if no image uploaded for this item, use a placeholder image instead
            product_image->scale_to_fit(128, 128);//resize(128, 128);//scale_to_fit(view->get_size());
            product_image->set_relative_position(box->get_x() + (box->get_width() - product_image->get_width()) / 2, heart_icon->get_relative_y() + heart_icon->get_height() + 10);//44//or 50);//box->get_y() + (box->get_height() - product_image->get_height()) / 2);//set_alignment("center"); // actual relative position is set in the draw call
            box->add_image(*product_image);
            // save the item_id as a component so we know which item_id belongs to which box
            box->add_component(* new Component("item_id", static_cast<int>(item_id))); // will be stored as shared_ptr by the entity class
            std::cout << "component item_id(" << box->get_component("item_id") << ") added to box " << i << std::endl;
            //////////////////////////////////////////////////
            // product name
            std::string product_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1;", { std::to_string(item_id) });//std::cout << "product_name: " << product_name << std::endl;
            dokun::Label * product_name_label = new dokun::Label(product_name);
            product_name_label->set_color(32, 32, 32, 1.0);
            product_name_label->set_relative_position(10, product_image->get_relative_y() + product_image->get_height() + 10);
            box->add_gui(*product_name_label);
            //////////////////////////////////////////////////
            // stars (just for display, users cannot rate products in the catalog_view, but they can on the product_page)
            float average_stars = item.get_average_stars();
            //std::cout << "average_stars for item(id: " << item_id << "): " << average_stars << std::endl;
            std::vector<Image *> product_stars; // size will be 5    //std::cout << "number of stars: " << product_stars.size() << std::endl;
            for(int i = 0; i < 5; i++) {
                product_stars.push_back(new Image());                
                product_stars[i]->load(Icon::get["star"]->get_data(), 64, 64, 1, 4);//"star_half"
                product_stars[i]->resize(20, 20);//(16, 16);
                product_stars[i]->set_color((average_stars > 0) ? 255, 179, 68, 1.0 : 255, 255, 255, 1.0);
                ////product_stars[i]->set_outline(true); // gives the star an illusion of depth
                ////product_stars[i]->set_outline_thickness(0.6);//(1.0);
                ////product_stars[i]->set_outline_color(230, 136, 0);// shades = rgb(230, 136, 0) = THE perfect outline color, rgb(179, 106, 0) = looks really bad//product_stars[i]->set_outline_threshold(0.0);
                product_stars[i]->set_visible(false);
                box->add_image(*product_stars[i]);// same as: current->set_image(*product_stars[0].get(), i); // except that Box::set_image uses insert() rather than push_back(). This is the only difference between Box::add_image and Box::set_image
                if(i == 0) { 
                    product_stars[0]->set_relative_position(10, product_name_label->get_relative_y() + product_name_label->get_height() + 10);//(box->get_width() - (product_stars[0]->get_width() * 5) - 10, product_name_label->get_relative_y() + (product_name_label->get_height() - product_stars[0]->get_height()) / 2); // set position of the first star (other stars will follow it)
                    continue; // skip the first star for now
                }
                // update positions of stars
                product_stars[i]->set_relative_position(product_stars[i - 1]->get_relative_x() + product_stars[i - 1]->get_width() + 1, product_stars[0]->get_relative_y()); // same y_rel_pos as first star
            }            
            // draw yellow stars on top of gray/white stars or ?
            // ...
            // product review label
            int product_ratings_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(*) FROM item_ratings WHERE item_id = $1", { std::to_string(item_id) });//item.get_ratings_count();
            dokun::Label * product_reviews_label = new dokun::Label("("+std::to_string(product_ratings_count)+")");//(star_ratings > 0) ? std::to_string(star_ratings)+" ratings" : "No ratings yet");
            product_reviews_label->set_color(16, 16, 16, 1.0);
            product_reviews_label->set_visible(product_stars[0]->is_visible());
            product_reviews_label->set_relative_position(product_stars[product_stars.size() - 1]->get_relative_x() + product_stars[product_stars.size() - 1]->get_width() + 1, product_stars[product_stars.size() - 1]->get_relative_y() + (product_stars[0]->get_height() - product_reviews_label->get_height()) / 2);
            box->add_gui(*product_reviews_label);            
            //////////////////////////////////////////////////
            // price display
            double sales_price = std::stod(PQgetvalue(result, i, 4));//std::cout << "product_price: " << sales_price << std::endl;
            std::string currency = PQgetvalue(result, i, 5);//std::cout << "seller currency: " << currency << std::endl;
            std::string currency_symbol = Converter::get_currency_symbol(currency);
            //double sales_price_xmr = Converter::to_xmr(sales_price, currency); // requires libcurl + internet connection (may fail at times) and also slows app launch
            double sales_price_xmr = 0.0; // this is only temporary
            //std::cout << "product_price_xmr: " << String::to_string_with_precision(sales_price_xmr, 12) << std::endl;
            dokun::Label * price_label = new dokun::Label(currency_symbol + String::to_string_with_precision(sales_price, 2) + " " + String::upper(currency));
            price_label->set_color(0, 0, 0, 1.0);
            if(product_stars[0]->is_visible()) price_label->set_relative_position(10, product_stars[0]->get_relative_y() + product_stars[0]->get_height() + 10);//box->get_height() - price_label->get_height() - 10);
            if(!product_stars[0]->is_visible()) price_label->set_relative_position(10, product_name_label->get_relative_y() + product_name_label->get_height() + 10);//15);
            box->add_gui(*price_label);
            // cryptocurrency ticker (symbol)
            Image * monero_symbol = new Image(Icon::get["monero_symbol_white"]->get_data(), 64, 64, 1, 4);
            monero_symbol->set_outline(true);
            monero_symbol->set_outline_thickness(0.2);//(1.2);
            monero_symbol->resize(16, 16);
            monero_symbol->set_visible(false);
            monero_symbol->set_relative_position(10, price_label->get_relative_y() + price_label->get_height() + 8);
            box->add_image(*monero_symbol);
            // price display xmr
            dokun::Label * price_label_xmr = new dokun::Label(String::to_string_with_precision(sales_price_xmr, 12) + " XMR");
            if(!monero_symbol->is_visible()) price_label_xmr->set_relative_position(10, price_label->get_relative_y() + price_label->get_height() + 10 );//(monero_symbol->get_relative_x() + monero_symbol->get_width() + 5, monero_symbol->get_relative_y() + (monero_symbol->get_height() - price_label->get_height()) / 2);// <= use this if monero_symbol is visible
            if(monero_symbol->is_visible()) price_label_xmr->set_relative_position(monero_symbol->get_relative_x() + monero_symbol->get_width() + 5, monero_symbol->get_relative_y() + (monero_symbol->get_height() - price_label->get_height()) / 2);
            price_label_xmr->set_color(0, 0, 0, 1.0);
            price_label_xmr->set_visible(false);
            box->add_gui(*price_label_xmr);
            //////////////////////////////////////////////////
            // tooltip
            //If(mouse_is_over(heart_icon)) tooltip->show("This item is in your favorites");
            //If(mouse_is_over(verified_purchase_icon)) tooltip->show("You've previously purchased this item");
            //////////////////////////////////////////////////
            // show_product or buy button (not neccessary, but optional since user can just click the box to view the item)
            // ...
        ////} // grid (view) columns
    } // grid (view) rows
    ////////////////////
    // in most cases, the number of rows (products) is greater than the box_count so this "if" scope will be ignored 99% of the time but in case all boxes are not filled with product images, fill the ones that need it
    if(rows < box_count) { // if there are less items than catalog boxes, get the boxes that do need an item and fill them up with placeholder product images
        for(int i = 0; i < box_count; i++) {//std::cout << "box_index(i): " << i << std::endl;
            Box * box = view->get_box(i);
            if(box->get_image_count() > 0) continue; // skip boxes that already have product images
            // product_image (thumbnail)
            Image * product_image = new Image(Icon::get["image_gallery"]->get_data(), 64, 64, 1, 4);
            product_image->scale_to_fit(128, 128);
            product_image->set_alignment("center");
            box->add_image(*product_image);
        }
    }
    ////////////////////
    PQclear(result); // free result
#endif    
}
////////////////////
void neroshop::Catalog::fetch_items_and_inventory() {
    std::string command = "SELECT DISTINCT ON (item.id) * FROM item JOIN inventory ON item.id = inventory.item_id ORDER BY item.id ASC LIMIT $1;";
}
////////////////////
void neroshop::Catalog::fetch_best_sellers() {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // if an item has never been ordered at least once, then it will not appear in best-sellers
    std::string command = "SELECT item_id FROM order_item GROUP BY item_id ORDER BY SUM(item_qty) DESC LIMIT $1;"; // DESC (from highest to lowest sum of item_qty)
    int box_count = view->get_row_count() * view->get_column_count(); // rows x columns
    std::vector<const char *> param_values = { std::to_string(box_count).c_str() };
    PGresult * result = PQexecParams(DB::Postgres::get_singleton()->get_handle(), command.c_str(), 1, nullptr, param_values.data(), nullptr, nullptr, 0);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        neroshop::print("Catalog::fetch(): No items found", 2);        
        PQclear(result);//DB::Postgres::get_singleton()->finish();//exit(1);
        return; // exit so we don't double free "result"
    }
    int rows = PQntuples(result); // max_rows is limited to number of boxes in the grid//std::cout << "number of rows (items): " << rows << std::endl;
    for(int i = 0; i < std::min<size_t>(rows, box_count); i++) {
        Box * box = view->get_box(i);//(r, c);
        int item_id = std::stoi(PQgetvalue(result, i, 0));
        std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_id) });
        std::cout << "best-selling item_ids: " << item_id << " (" << item_name << ")" << std::endl;
        // draw contents here ...
        // best_seller ticker
        /*Box * best_seller_ticker = new Box();
        best_seller_ticker->set_size(120, 24);
        best_seller_ticker->set_color(255, 82, 82, 1.0);//https://www.color-hex.com/color-palette/2539
        best_seller_ticker->set_radius(10);////dokun::Font * font = new dokun::Font(); font->set_pixel_size(0, 14); font->load(DOKUN_DEFAULT_FONT_PATH);////best_seller_ticker->set_size(110 - 20, 17 + 3);//std::cout << "text size(based on font calc): " << Vector2(font->get_width("best seller"), font->get_height("best seller")) << std::endl;
        dokun::Label * best_seller_label = new dokun::Label("best seller");//new dokun::Label(*font, "best seller");
        best_seller_label->set_alignment("center");//best_seller_label->set_relative_position((best_seller_ticker->get_width() - font->get_width("best seller")) / 2, (best_seller_ticker->get_height() - font->get_height("best seller")) / 2);
        best_seller_ticker->set_label(*best_seller_label);//or add_gui//best_seller_ticker->set_size(best_seller_label->get_width() + 20, 24);
        //best_seller_ticker->set_visible(true);
        best_seller_ticker->set_relative_position(40, 10); // center the x or nah?
        box->add_gui(*best_seller_ticker);*/        
    }
    ////////////////////
    PQclear(result); // free result when done using it
#endif    
}
////////////////////
// I don't know how to get the mode of unnest(item_ids) in table favorites :/
void neroshop::Catalog::fetch_most_favorited() {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // if an item has never been favorited at least once, then it will not appear in most favorited
    std::string command = "";//"SELECT unnest(item_ids) FROM favorites;";
    /*SELECT count(*), UNNEST(item_ids) as item_id
    FROM favorites GROUP BY item_id;*/
    int box_count = view->get_row_count() * view->get_column_count(); // rows x columns
    std::vector<const char *> param_values = { std::to_string(box_count).c_str() };
    PGresult * result = PQexecParams(DB::Postgres::get_singleton()->get_handle(), command.c_str(), 1, nullptr, param_values.data(), nullptr, nullptr, 0);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        neroshop::print("Catalog::fetch(): No items found", 2);        
        PQclear(result);//DB::Postgres::get_singleton()->finish();//exit(1);
        return; // exit so we don't double free "result"
    }
    int rows = PQntuples(result); // max_rows is limited to number of boxes in the grid//std::cout << "number of rows (items): " << rows << std::endl;
    for(int i = 0; i < std::min<size_t>(rows, box_count); i++) {
        Box * box = view->get_box(i);//(r, c);
        int item_id = std::stoi(PQgetvalue(result, i, 0));
        std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_id) });
        std::cout << "most favorited item_ids: " << item_id << " (" << item_name << ")" << std::endl;
        // draw contents here ...
    }
    ////////////////////
    PQclear(result); // free result when done using it
#endif    
}
////////////////////
// refresh content (when user logs in or performs an item search) without allocating new objects
void neroshop::Catalog::refresh(const neroshop::User& user) {
    // retrieve the cart from the user and store it (for reading only)
    if(!cart) cart = user.get_cart();
    // if catalog is empty (all boxes without any content), populate it
    // ...
    // refresh catalog with updated information
    // ...
    //-----------------------------------
    neroshop::print("catalog refreshed");    
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
