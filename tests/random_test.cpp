#include <iostream>
#include <string>
// neroshop
#include "../include/neroshop.hpp"
using namespace neroshop;

static void start_and_initialize_database() {
    std::cout << "sqlite3 v" << DB::SQLite3::get_sqlite_version() << std::endl;
    DB::SQLite3 * database = DB::SQLite3::get_singleton();//std::unique_ptr<DB::SQLite3> database = std::make_unique<DB::SQLite3>();
    if(!database->open("data.sqlite3")) {//(":memory:")) { // In-memory databases are temporary and written in RAM. They do not support WAL mode either
        neroshop::print(SQLITE3_TAG "\033[91mSQLite::open failed");
    }
    //-------------------------
    // Create a table and add columns
    /*if(!database->table_exists("users")) {
        std::cout << "CREATE TABLE users\n";
        database->execute("CREATE TABLE IF NOT EXISTS users(id  INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        database->execute("ALTER TABLE users ADD name text;");
        //database->execute("ALTER TABLE users ADD pw_hash text;");
        database->execute("ALTER TABLE users ADD age integer;");
    }*/
    //database->execute();
    // Insert a new row
    ////database->execute_params("INSERT INTO users (name, age) VALUES ($1, $2);", { "sid", "25" });
    ////std::cout << database->get_text_params("SELECT name FROM users WHERE id = $1", { "1" }) << std::endl;
    // Modify new row
    //database->execute_params("SELECT * FROM users WHERE id = $1 AND name = $2", { "1", "dude" });
    //-------------------------
    // todo: store this code in a initialize_database function
    database->execute("BEGIN;");
    //-------------------------
    // table: account_type
    /*if(!database->table_exists("account_type")) {
        database->execute("CREATE TABLE account_type(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        database->execute("ALTER TABLE account_type ADD COLUMN name TEXT");
        database->execute("INSERT INTO account_type(name) VALUES('Buyer');");
        database->execute("INSERT INTO account_type(name) VALUES('Seller');");
    }*/    
    //-------------------------
    // users
    // if 2 users share the same id or name then the raft consensus protocol must decide whether t
    if(!database->table_exists("users")) { // todo: rename "users" to "peers" or create a separate peers table
        database->execute("CREATE TABLE users(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        ////database->execute("ALTER TABLE users ADD COLUMN name TEXT;"); // mostly suitable for centralized or federated apps
        database->execute("ALTER TABLE users ADD COLUMN verify_key TEXT;"); // verify_key - public_key used for verification of signatures
        database->execute("ALTER TABLE users ADD COLUMN encrypt_key TEXT;"); // encrypt_key - public_key used for encryption of messages
        ////database->execute("CREATE UNIQUE INDEX index_user_names ON users (name);");
        database->execute("CREATE UNIQUE INDEX index_verify_keys ON users (verify_key);");//database->execute("CREATE UNIQUE INDEX index_users ON users (name, verify_key, encrypt_key);");// enforce that the user names are unique, in case there is an attempt to insert a new "name" of the same value
        database->execute("CREATE UNIQUE INDEX index_encrypt_keys ON users (encrypt_key);");//database->execute("CREATE UNIQUE INDEX index_users ON users (name, verify_key, encrypt_key);");// enforce that the user names are unique, in case there is an attempt to insert a new "name" of the same value
        // sellers require a username and buyers do not
        // passwords will no longer be stored as salted bcrpyt hashes, only public keys will be stored
    }
    //-------------------------
    // products (items)
    if(!database->table_exists("products")) {
        database->execute("CREATE TABLE products(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        database->execute("ALTER TABLE products ADD COLUMN name TEXT;");
        database->execute("ALTER TABLE products ADD COLUMN description TEXT;"); // desc or description
        database->execute("ALTER TABLE products ADD COLUMN price REAL");//INTEGER;");//numeric(20, 12);"); // unit_price or price_per_unit // 64-bit integer (uint64_t) that will be multipled by a piconero to get the actual monero price
        database->execute("ALTER TABLE products ADD COLUMN weight REAL;"); // kg
        //database->execute("ALTER TABLE products ADD COLUMN size ?datatype;"); // l x w x h
        database->execute("ALTER TABLE products ADD COLUMN code TEXT;"); // product_code can be either upc (universal product code) or a custom sku
        //database->execute("ALTER TABLE products ADD COLUMN category_id INTEGER REFERENCES categories(id);");
        //database->execute("ALTER TABLE products ADD COLUMN subcategory_id INTEGER REFERENCES categories(id);");
        //database->execute("ALTER TABLE products ADD COLUMN ?col ?datatype;");
        database->execute("CREATE UNIQUE INDEX index_product_codes ON products (code);"); // product codes must be unique
        // the seller determines the product condition and whether the product will have a discount or not
    }
    //-------------------------
    // inventory
    if(!database->table_exists("inventory")) {
        database->execute("CREATE TABLE inventory(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        database->execute("ALTER TABLE inventory ADD COLUMN item_id INTEGER REFERENCES products(id);");
        database->execute("ALTER TABLE inventory ADD COLUMN seller_id INTEGER REFERENCES users(id);"); // alternative names: "store_id"
        database->execute("ALTER TABLE inventory ADD COLUMN stock_qty INTEGER;"); // alternative names: "stock" or "stock_available"
        database->execute("ALTER TABLE inventory ADD COLUMN sales_price REAL;");//numeric(20,12);"); // alternative names: "seller_price" or "list_price" // this is the final price of a product
        database->execute("ALTER TABLE inventory ADD COLUMN currency TEXT;"); // the fiat currency the seller is selling the item in
        //database->execute("ALTER TABLE inventory ADD COLUMN discount numeric(20,12);"); // alternative names: "seller_discount", or "discount_price"
        //database->execute("ALTER TABLE inventory ADD COLUMN ?col ?datatype;"); // discount_times_can_use - number of times the discount can be used
        //database->execute("ALTER TABLE inventory ADD COLUMN ?col ?datatype;"); // discounted_items_qty - number of items that the discount will apply to 
        //database->execute("ALTER TABLE inventory ADD COLUMN ?col ?datatype;"); // discount_expiry -  date and time that the discount expires (will be in UTC format)
        //database->execute("ALTER TABLE inventory ADD COLUMN ?col ?datatype;");        
        database->execute("ALTER TABLE inventory ADD COLUMN condition TEXT;"); // item condition
        //database->execute("ALTER TABLE inventory ADD COLUMN last_updated ?datatype;");
        //database->execute("");
    }    
    //-------------------------
    // cart
    if(!database->table_exists("cart")) {
        // local cart - for a single cart containing a list of item_ids
        //database->execute("CREATE TABLE cart(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, item_id INTEGER REFERENCES products(id));");
        // public cart - copied to all peers' databases
        database->execute("CREATE TABLE cart(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        database->execute("ALTER TABLE cart ADD COLUMN user_id INTEGER REFERENCES users(id);");//database->execute("CREATE TABLE cart(id INTEGER NOT NULL PRIMARY KEY, user_id INTEGER REFERENCES users(id));");
        // cart_items (public cart)
        database->execute("CREATE TABLE cart_item(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        database->execute("ALTER TABLE cart_item ADD COLUMN cart_id INTEGER REFERENCES cart(id);");
        database->execute("ALTER TABLE cart_item ADD COLUMN item_id INTEGER REFERENCES products(id);");
        database->execute("ALTER TABLE cart_item ADD COLUMN item_qty INTEGER;");
        //database->execute("ALTER TABLE cart_item ADD COLUMN item_price numeric;");
        //database->execute("ALTER TABLE cart_item ADD COLUMN item_weight REAL;");//database->execute("CREATE TABLE cart_item(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, cart_id INTEGER REFERENCES cart(id), item_id INTEGER REFERENCES products(id), item_qty INTEGER, item_price NUMERIC, item_weight REAL);");
    }
    //-------------------------
    // orders (purchase_orders)
    if(!database->table_exists("orders")) {
        database->execute("CREATE TABLE orders(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");//database->execute("ALTER TABLE orders ADD COLUMN ?col ?datatype;");
        database->execute("ALTER TABLE orders ADD COLUMN timestamp TEXT DEFAULT CURRENT_TIMESTAMP;"); // creation_date // to get UTC time: set to datetime('now');
        database->execute("ALTER TABLE orders ADD COLUMN status TEXT;");
        database->execute("ALTER TABLE orders ADD COLUMN user_id INTEGER REFERENCES users(id);"); // the user that placed the order
        //database->execute("ALTER TABLE orders ADD COLUMN weight REAL;"); // weight of all order items combined - not essential
        database->execute("ALTER TABLE orders ADD COLUMN subtotal numeric(20, 12);");
        database->execute("ALTER TABLE orders ADD COLUMN discount numeric(20, 12);");
        //database->execute("ALTER TABLE orders ADD COLUMN shipping_method TEXT;"); // comment this out
        database->execute("ALTER TABLE orders ADD COLUMN shipping_cost numeric(20, 12);");
        database->execute("ALTER TABLE orders ADD COLUMN total numeric(20, 12);");
        //database->execute("ALTER TABLE orders ADD COLUMN notes TEXT;"); // will contain sensative such as shipping address and tracking numbers that will be encrypted and can only be decrypted by the seller - this may not be necessary since buyer can contact seller privately
        // order_item
        database->execute("CREATE TABLE order_item(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        database->execute("ALTER TABLE order_item ADD COLUMN order_id INTEGER REFERENCES orders(id);");
        database->execute("ALTER TABLE order_item ADD COLUMN item_id INTEGER REFERENCES products(id);");
        database->execute("ALTER TABLE order_item ADD COLUMN seller_id INTEGER REFERENCES users(id);");
        database->execute("ALTER TABLE order_item ADD COLUMN item_qty INTEGER;");
        //database->execute("ALTER TABLE order_item ADD COLUMN item_price ?datatype;");
        //database->execute("ALTER TABLE order_item ADD COLUMN ?col ?datatype;");
    }
    //-------------------------
    // ratings - product_ratings, seller_ratings
    // maybe merge both item ratings and seller ratings together or nah?
    if(!database->table_exists("seller_ratings")) {//if(!database->table_exists("user_ratings")) {
        // seller
        database->execute("CREATE TABLE seller_ratings(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        database->execute("ALTER TABLE seller_ratings ADD COLUMN seller_id INTEGER REFERENCES users(id);"); // seller_pkey or seller_pubkey//database->execute("ALTER TABLE user_ratings ADD COLUMN user_id INTEGER REFERENCES users(id);"); // seller_pkey or seller_pubkey
        database->execute("ALTER TABLE seller_ratings ADD COLUMN score INTEGER;");
        database->execute("ALTER TABLE seller_ratings ADD COLUMN user_id INTEGER REFERENCES users(id);"); // or rater_id // user_pkey or user_pubkey//database->execute("ALTER TABLE user_ratings ADD COLUMN rater_id INTEGER REFERENCES users(id);"); // user_pkey or user_pubkey
        database->execute("ALTER TABLE seller_ratings ADD COLUMN comments TEXT;"); // plain text
    }
    if(!database->table_exists("product_ratings")) {
        // product - put in a separate table reviews (but revies only mean comments)
        database->execute("CREATE TABLE product_ratings(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        database->execute("ALTER TABLE product_ratings ADD COLUMN item_id INTEGER REFERENCES products(id);");
        database->execute("ALTER TABLE product_ratings ADD COLUMN stars INTEGER;");
        database->execute("ALTER TABLE product_ratings ADD COLUMN user_id INTEGER REFERENCES users(id);");
        database->execute("ALTER TABLE product_ratings ADD COLUMN comments TEXT;"); // plain text
    }
    //-------------------------
    // images
    if(!database->table_exists("images")) {
        //database->execute("CREATE TABLE images(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        //database->execute("ALTER TABLE images ADD COLUMN item_id INTEGER REFERENCES products(id);");
        //database->execute("ALTER TABLE images ADD COLUMN name TEXT[];");
        //database->execute("ALTER TABLE images ADD COLUMN data BLOB[];");
        //database->execute("ALTER TABLE images ADD COLUMN ?col ?datatype;");
        //database->execute("ALTER TABLE images ADD COLUMN ?col ?datatype;");
    }    
    //-------------------------
    // avatars - each user will have a single avatar
    if(!database->table_exists("avatars")) {
        //database->execute("CREATE TABLE avatars(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        //database->execute("ALTER TABLE avatars ADD COLUMN user_id INTEGER REFERENCES users(id);");
        //database->execute("ALTER TABLE avatars ADD COLUMN data BLOB;");
    }    
    //-------------------------
    // product_categories, product_subcategories
    if(!database->table_exists("categories")) {
        database->execute("CREATE TABLE categories(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        database->execute("ALTER TABLE categories ADD COLUMN name TEXT;");
        database->execute("ALTER TABLE categories ADD COLUMN alt_names TEXT;");
        //database->execute("ALTER TABLE categories ADD COLUMN desc ?datatype;");
    }    
    if(!database->table_exists("subcategories")) {
        database->execute("CREATE TABLE subcategories(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        //database->execute("ALTER TABLE subcategories ADD COLUMN ?col ?datatype;");
        database->execute("ALTER TABLE subcategories ADD COLUMN name TEXT;");
        database->execute("ALTER TABLE subcategories ADD COLUMN category_id INTEGER REFERENCES categories(id);");
        database->execute("ALTER TABLE subcategories ADD COLUMN description TEXT;");
    }        
    // insert all categories and subcategories
             // category here
             int category_id = database->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Food', 'Grocery & Gourmet Foods; Produce') RETURNING id;"); // Almost done :) // Food & Beverage in one category? nah
             // subcategories here
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Canned Foods', $1)", { std::to_string(category_id) }); // sub subcategory
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Condiments', $1)", { std::to_string(category_id) }); // sweet and savory sauces // includes ketchup, mustard, mayonnaise, sour cream, barbecue sauce, compound butter, etc. // A condiment is a preparation that is added to food, typically after cooking, to impart a specific flavor, to enhance the flavor, or to complement the dish 
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Confections', $1)", { std::to_string(category_id) }); // confections or confectionery are sweets like cake, deserts, candies, marshmellows, and other food items that are sweet // sugar confections (aka sweets) = candy, baker/flour confection = cake, chocolate confection = hershey
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Dairy', $1)", { std::to_string(category_id) }); // includes food items like cheese, milk, and yogurt
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Fish & Seafood', $1)", { std::to_string(category_id) }); // includes fish, crab, and all other seafoods
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Food Additives - Sweeteners', $1)", { std::to_string(category_id) }); // includes honey, maple syrup, raw sugar, etc.             
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Frozen Foods', $1)", { std::to_string(category_id) }); // sub subcategory
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Fruits', $1)", { std::to_string(category_id) }); // includes berries, melons, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Grains, Breads & Cereals', $1)", { std::to_string(category_id) }); // includes cereal, rice, oatmeal, pasta and traditional bakery like biscuits and bread (whole grains)
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Herbs & Spices', $1)", { std::to_string(category_id) }); //include ginger, turmeric, cinnamon, cocoa powder, parsley, and basil, etc. // herbs are leafy things, like basil, tarragon, thyme, and cilantro; and spices are seeds, either whole or ground, like coriander, cumin, and anise. Leaves vs. seeds is indeed a simple version of the separation. But simple isn't always accurate.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Meat & Poutry', $1)", { std::to_string(category_id) }); //Protein -// All poutry is meat, well since a bird and chicken are meat // fish should have its own subcategory // pescatarians - vegetarians who eat fish and no other flesh or meat
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Nuts, Seeds & Legumes', $1)", { std::to_string(category_id) }); // a nut is a hard-shell containing a seed and a seed is without a shell // legumes are beans
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Fats, Oils, Sugar & Salt', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Organic Foods', $1)", { std::to_string(category_id) }); // sub subcategory
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Processed Foods', $1)", { std::to_string(category_id) }); // sub subcategory
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Proteins - Eggs', $1)", { std::to_string(category_id) }); // eggs are neither dairy (since they don't come from milk, but laid by chickens) nor meat (eggs contain no animal flesh since they are unfertilized)
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Snack Foods', $1)", { std::to_string(category_id) }); // pretzels, slated peanuts, potato chips, popcorn, trail mix, candy, chocolate, baked sweets, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Vegetables', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Vitamins & Dietary Supplements', $1)", { std::to_string(category_id) }); //Nutritional and herbal supplements // includes bee pollen, matcha powder, protein powder, and powdered vitamin or mineral supplements, etc. // Dietary supplements can include a variety of components such as vitamins, minerals, herbs, amino acids, enzymes and other ingredients taken in various forms such as a pill, gel, capsule, gummy, powder, drink or food
             ////database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = database->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Beverages', '') RETURNING id;"); // DONE! :D
             // subcategories here
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Alchoholic Beverages - Hard Liquors & Spirits', $1)", { std::to_string(category_id) }); // spirits is another name for liquor (almost), both are distilled beverages and are usually put into these two categories: liquors and spirits // distilled rather than fermented alcoholic beverage
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Alchoholic Beverages - Beer', $1)", { std::to_string(category_id) }); // beer is made from fermented grain
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Alchoholic Beverages - Wine', $1)", { std::to_string(category_id) }); // most wine is produced by fermenting grape juice. However, you can’t just use any type of grape. You need wine grapes
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Coffee', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Juices', $1)", { std::to_string(category_id) }); // lemonade included :} // lemonade could also be a carbonated drink :O // is smoothie a juice? :O
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Milk or milk-based', $1)", { std::to_string(category_id) }); // milkshake, most hot chocolate and some protein shake included :}
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Mixed drinks', $1)", { std::to_string(category_id) }); // cocktail is a mixed drink with a mix of alcohol, fruit juice, soda, etc. :O // what about smoothies? :O
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Soft drinks', $1)", { std::to_string(category_id) }); // soda
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Sports & Energy drinks', $1)", { std::to_string(category_id) }); // sports drinks like gatorade and energy drinks like red bull
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Tea', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Water - Carbonated', $1)", { std::to_string(category_id) }); // same thing as sparkling water
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Water - Non-carbonated', $1)", { std::to_string(category_id) }); // regular drinking water
             //---------------------------
             // category here
             category_id = database->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Electronics', '') RETURNING id;"); // aka Consumer Electronics or Hardware
             // subcategories here
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances', $1)", { std::to_string(category_id) }); // Home Appliances
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Dishwashers', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Garbage Disposals & Compactors', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Heating, Cooling & Air Quality Appliances', $1)", { std::to_string(category_id) }); // air conditioners, heaters             
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Home Appliance Warranties', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Kitchen Small Appliances', $1)", { std::to_string(category_id) }); // blenders, toasters, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Laundry Appliances', $1)", { std::to_string(category_id) }); // washing machines, dryers, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Microwave Ovens', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Parts & Accessories', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Ranges, Ovens & Cooktops', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Refrigerators, Freezers & Ice Makers', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Outdoor Appliances', $1)", { std::to_string(category_id) }); // grills, smokers
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Vacuums & Floor Care Appliances', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Automotive Electronics - ', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Automotive Electronics', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Automotive Electronics - GPS Navigators', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Automotive Electronics - ', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Automotive Electronics - Radio and Vehicle audio', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Automotive Electronics - ', $1)", { std::to_string(category_id) });
             //////database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Accessories', $1)", { std::to_string(category_id) }); // accessories for electronics
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('AC Adapters, Chargers, Power strips, & Converters', $1)", { std::to_string(category_id) }); // includes chargers, power outlet (wall) chargers, universal power adapters / Travel Adapters & Converters, power strips, voltage converters, power banks, etc.
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Cables, Cords, Plugs, and Wires', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Cables and Wires - Monitor Cables', $1)", { std::to_string(category_id) }); // usb-c, hdmi, vga, dvi, displayport, rca or component/composite video, thunderbolt 1/2, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Cables -', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Calculators', $1)", { std::to_string(category_id) }); // including scientific ones
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Camera & Photo - Camcorders and Digital Cameras', $1)", { std::to_string(category_id) }); // includes digital cameras(DSLR,  Mirrorless,  Compact Cameras / Point & Shoot Cameras, Bridge Cameras, Instant Cameras, Film Cameras, Action Cameras, 360 Degree Cameras, Underwater Cameras / Waterproof Cameras, Medium Format Cameras, SmartPhone Cameras, Rangefinder Cameras, Security Camera, Smartphone Camera, Drone Camera, etc.)
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Camera & Photo - Display Stands', $1)", { std::to_string(category_id) }); // includes digital cameras(DSLR,  Mirrorless,  Compact Cameras / Point & Shoot Cameras, Bridge Cameras, Instant Cameras, Film Cameras, Action Cameras, 360 Degree Cameras, Underwater Cameras / Waterproof Cameras, Medium Format Cameras, SmartPhone Cameras, Rangefinder Cameras, Security Camera, Smartphone Camera, Drone Camera, etc.)             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Camera & Photo - Film Cameras', $1)", { std::to_string(category_id) }); // traditional cameras with film - still digital
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Camera & Photo - Security cameras', $1)", { std::to_string(category_id) }); // includes doorbell cameras, security cameras such as bullet, dome, hidden/covert, infrared, box, outdoor, PTZ, and wireless
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Cell Phones - Mobile Phones & Smartphones', $1)", { std::to_string(category_id) }); // aka Mobile phones//database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Phones - Telephones', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Clocks - Digital clocks & Alarm clocks', $1)", { std::to_string(category_id) }); // includes alarm clocks
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Computers - Desktops', $1)", { std::to_string(category_id) }); // aka PC
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Computers - Laptops', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Computers - Laptop Tablet Hybrids', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Computer Parts & Hardware', $1)", { std::to_string(category_id) }); // or Computer Hardware
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Electronic Repair Tools', $1)", { std::to_string(category_id) }); // drills, etc.
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Electric Shavers, Razors & Curling Irons', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - 3D Glasses', $1)", { std::to_string(category_id) }); // Entertainment or TV & Theater
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - CD, DVD and Blu-ray players', $1)", { std::to_string(category_id) }); // Blu-ray, is a digital optical disc storage format. It is designed to supersede the DVD format
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - ', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - ', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - Portable audio players & Boomboxes', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - Radios', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - ', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - ', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - Remote Controllers', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - Stereo systems', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - TVs', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - TV Receivers', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - ', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - Video Cassette Recorders (VCR)', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Electronic Musical Instruments', $1)", { std::to_string(category_id) }); // includes Audio Interfaces, Electronic Drums, Digital Synthesizers and Midi Controllers, Microphones, Studio Monitors, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });   
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Hardware - ', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Headphones, Earphones & Earbuds', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Industrial & Scientific - Electronic Measurement Devices', $1)", { std::to_string(category_id) }); // scales, calipers, etc.  // Industrial & Scientific will have its own main category                     
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });
             ////database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Lighting - Lamps, Lightbulbs', $1)", { std::to_string(category_id) }); // reading lamps, etc. // put this in Home Tools & Improvements category
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Lawn Mowers', $1)", { std::to_string(category_id) }); // reading lamps, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Headsets', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Microphone', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Mouse', $1)", { std::to_string(category_id) }); // dont forget touchpad and pen input, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Keyboards', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Monitors', $1)", { std::to_string(category_id) }); // screen - led, lcd, ctr, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Power Adapters', $1)", { std::to_string(category_id) }); // laptop power adaptors
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Projectors', $1)", { std::to_string(category_id) }); // screen
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Speakers', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - USB Hubs', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Webcams', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - ', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Hardware - Ports, Jacks, Slots, Sockets & Connectors', $1)", { std::to_string(category_id) }); // includes audio jacks, usb ports, microSD card reader, etc. // all plugs and ports are connectors
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Portable Media Devices', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Portable Media Devices - ', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Portable Media Devices - CB & Two-way Radio', $1)", { std::to_string(category_id) }); // walkie-talkies             
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Portable Media Devices - ', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Portable Media Devices - Media Players', $1)", { std::to_string(category_id) }); // portable mp3 players, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Portable Media Devices - ', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Printers - 3D Printers', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Printers - Inkjet Printers', $1)", { std::to_string(category_id) }); // single function printers only print while multi-function printers can print, scan, copy, and fax
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Printers - Laser Printers', $1)", { std::to_string(category_id) });             
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Printers, Scanners, Copiers, & Fax machines', $1)", { std::to_string(category_id) }); // single function printers only print while multi-function printers can print, scan, copy, and fax
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Ink and toner cartridges', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Safe box', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media', $1)", { std::to_string(category_id) }); // includes hard disk drives (hdd), solid state drives (ssd), m.2 ssd, network attached storage (nas), etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media - Cassette Tapes', $1)", { std::to_string(category_id) }); // small ones for audio, big ones for video
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media - CDs/DVDs/Blu-rays', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media - Floppy Disks', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media - Game Discs & ROM Catridges', $1)", { std::to_string(category_id) }); // aka game cartridge, cart, or card
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media - Hard disk drives (HDD), Solid state drives (SSD)', $1)", { std::to_string(category_id) }); // includes hdd, sdd, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media - Memory Cards', $1)", { std::to_string(category_id) }); // includes sd cards, card readers
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media - USB Flash Drives', $1)", { std::to_string(category_id) }); // flash memory
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Tablets', $1)", { std::to_string(category_id) }); // includes ipad, android tablets, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Tablets - Graphics and Drawing Tablets', $1)", { std::to_string(category_id) }); // includes pen tablets (without a screen) and graphics tablets like wacom, etc. // https://www.autopano.net/types-of-drawing-tablets/
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Toys & Games', $1)", { std::to_string(category_id) }); // toys and games should have a category of its own
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Toys & Games - Electronic Toys', $1)", { std::to_string(category_id) });// includes electric car toys, remote control cars, drones, robots, etc. // toys and games should have a category of its own
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Video Game Consoles, Games & Accessories', $1)", { std::to_string(category_id) });  // includes video game controllers, remote controllers, headsets, virtual reality headsets (PSVR), covers, etc.
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wearable Technology', $1)", { std::to_string(category_id) });             
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wearable Technology - Body Mounted Cameras', $1)", { std::to_string(category_id) });      
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wearable Technology - Clips, Arm & Wristbands', $1)", { std::to_string(category_id) });      
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wearable Technology - Glasses', $1)", { std::to_string(category_id) });             
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wearable Technology - Rings', $1)", { std::to_string(category_id) });             
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wearable Technology - Smartwatches', $1)", { std::to_string(category_id) });             
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wearable Technology - Virtual Reality Gear & Headsets', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = database->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Digital Goods', '') RETURNING id;"); // DONE :D! // SELECT * FROM subcategories WHERE category_id = $1;--ORDER BY name;
             // subcategories here
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Codes - Cheat codes and hacks', $1)", { std::to_string(category_id) }); // replace "Codes" with "Information" anaa? :O // includes hacked information and leaks
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Codes - Discount and coupons', $1)", { std::to_string(category_id) }); // discounts, coupons/vouchers, promo codes etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Codes - Gift cards', $1)", { std::to_string(category_id) }); // gift certificates included
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Codes - Membership and subscription cards', $1)", { std::to_string(category_id) }); // PSN Membership card, PS+ subscription card, Xbox Live cards, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Codes - Prepaid cards', $1)", { std::to_string(category_id) }); // A prepaid card is not linked to a bank checking account or to a credit union share draft account. Instead, you are spending money you placed in the prepaid card account in advance. This is sometimes called “loading money onto the card”.// For hacked credit cards, this subcategory can be used >:D JK!
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Digital Art', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Digital Currencies - Cryptocurrencies', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Digital Currencies - Fiat', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('eBooks', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Freelancing', $1)", { std::to_string(category_id) }); // sell your personal services online in a gig-economy and work from home // freelancing is different and better than remote work cuz you are your own boss // Gigs are also known as “freelancing” and “independent contracting” jobs
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Information - Online accounts', $1)", { std::to_string(category_id) }); // includes online accounts such as video game accounts, netflix accounts, etc.        
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Media - Audio', $1)", { std::to_string(category_id) }); // sound, video, text, etc. // Multimedia is the combined use of sound, video, and text to present an idea
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Media - Documents', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Media - Photos', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Media - Videos', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Software', $1)", { std::to_string(category_id) }); // general software
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Video Games', $1)", { std::to_string(category_id) }); // video games are softwares with their own category :O
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Virtual Goods - In-Game content', $1)", { std::to_string(category_id) });  // video game/virtual avatars, items, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Virtual Goods - NFTs', $1)", { std::to_string(category_id) }); // NFTs can be more than just art ranging from Fashion and wearables, DeFi, Events and ticketing, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Websites and Domain names', $1)", { std::to_string(category_id) });
             ////database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = database->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Books', '') RETURNING id;");
             // subcategories here
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Children''s Books', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Comics & Manga', $1)", { std::to_string(category_id) }); // include Manga?
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Cookbooks', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Journals', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Magazines', $1)", { std::to_string(category_id) });             
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Newspapers', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Poetry', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Textbooks', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Novels', $1)", { std::to_string(category_id) });             
             ////database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = database->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Apparel', 'Clothing, Shoes and Accessories; Fashion') RETURNING id;"); // Wearables // Almost DONE :)!
             // subcategories here
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Armwear', $1)", { std::to_string(category_id) }); // gloves, bracelets, sleeves, armband, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Baby & Toddler', $1)", { std::to_string(category_id) }); // diapers, bibs, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Bags, Handbags, Backpacks, Wallets, Purses & Pouches', $1)", { std::to_string(category_id) }); // backpacks, handbags, purses, totes, laptop bags, duffel bags, fanny packs/belt bags, briefcases, etc. // https://shilpaahuja.com/types-of-bags/  // Carry Goods or Carry Access.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Bottomwear', $1)", { std::to_string(category_id) }); // pants/trousers/jeans, shorts, skirts, miniskirts, leggings, breeches, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Cookwear - Aprons & Bibs', $1)", { std::to_string(category_id) }); // kappōgis, pinafores, etc.             
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Costumes & Cosplay', $1)", { std::to_string(category_id) }); // costumes, reanactment and theater, and masks too
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Dresses & Gowns', $1)", { std::to_string(category_id) }); // dresses, blouses, gowns
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Eyewear', $1)", { std::to_string(category_id) }); // eyeglasses, googles, sunglasses / shades, monocles, eyepatches, contact lenses, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Footwear', $1)", { std::to_string(category_id) }); // socks, boots, shoes / sneakers, slippers, sandals, heels, flats, crocs             
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Handwear - Gloves, Mittens', $1)", { std::to_string(category_id) }); // gloves, mittens, boxing gloves, rubber gloves, knuckle dusters, etc.          
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Headwear', $1)", { std::to_string(category_id) }); // hats, caps, bandanas, headbands, crowns, wigs, kerchiefs, helmets, hijabs, hoods, headscarfs, turbans, hair nets, face masks, theatre masks, gas masks, sports mask for hockey, masquerade ball masks, ritual masks, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Hosiery - Socks & stockings, Pantyhose, Tights, etc.', $1)", { std::to_string(category_id) }); // socks & stockings, knee-highs, pantyhose, tights, fishnets + garters, leg warmers, yoga pants, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Kids'' Wear', $1)", { std::to_string(category_id) }); // or Children''s
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Knee clothing', $1)", { std::to_string(category_id) }); // knee-pads, etc.             
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Men''s Wear', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Neckwear', $1)", { std::to_string(category_id) }); // necklaces, chokers / collars, scarfs, neckties
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Outerwear', $1)", { std::to_string(category_id) }); // coats, sweaters / hoodies, and jackets
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Pets - Pets'' clothing', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Robes and cloaks', $1)", { std::to_string(category_id) }); // sleeping gowns, bath robes, ponchos, academic dress, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Scarfs, Shawls, Wraps & Pashimas', $1)", { std::to_string(category_id) }); // scarfs, shawls and wraps, pashimas
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Sleepwear & Pajamas', $1)", { std::to_string(category_id) }); // pyjamas, pyjama pants, night gowns, sleep shirts, etc. // Nightwear
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Sportswear', $1)", { std::to_string(category_id) }); // jerseys, swimsuits, tracksuits, bathing suits, wet suits, ski wear, motorcycle gear, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Suits & One-piece suits', $1)", { std::to_string(category_id) }); // one-piece suits include: Sling swimsuit, jumpsuits, flight/space suits, ski suits, chemturion, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Textiles', $1)", { std::to_string(category_id) }); // cloth pieces, fabrics:knitted, woven, and non-woven,  silk, cotton, leather, etc. // https://www.masterclass.com/articles/28-types-of-fabrics-and-their-uses#28-different-types-of-fabric  // 
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Topwear - Shirts & Tops', $1)", { std::to_string(category_id) }); // tees, tank-tops/singlets, vests, coats, sweaters, and jackets, cardigans
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Topwear', $1)", { std::to_string(category_id) }); // tees, tank-tops/singlets, vests, coats, sweaters, and jackets, cardigans             
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Traditional Wear', $1)", { std::to_string(category_id) }); // kimonos, kilts, sari(s), hwarots, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Underwear', $1)", { std::to_string(category_id) }); // slips, panties and boxers, bras, other undergarments
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Uniforms & Work Clothing', $1)", { std::to_string(category_id) }); // school uniforms, police uniforms, etc.
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Unisex', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Waistwear - Belts & Suspenders', $1)", { std::to_string(category_id) }); // belts, sashes like obi, karate belts, etc.             
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Watches, Rings & Jewelry', $1)", { std::to_string(category_id) }); // accessory: rings, earrings, watches, bracelets, chains, necklaces, pendants, charms, jewelry clasps / hooks, 
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wedding & Formal', $1)", { std::to_string(category_id) }); // wedding gowns, dress shirts/men's suits/sweater vest/waistcoat/lounge jackets(suit coats) // formal attire
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Women''s Wear', $1)", { std::to_string(category_id) });
             ////database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = database->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Pets', 'Domesticated Animals') RETURNING id;");
             // subcategories here
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Dogs', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Cats', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Birds', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Fishes', $1)", { std::to_string(category_id) });
             database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Reptiles', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Pet Supplies', $1)", { std::to_string(category_id) });
             ////database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = database->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Cosmetics', 'Beauty & Personal Care') RETURNING id;"); // skin and hair care
             // subcategories here
             //////database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             /*//---------------------------
             // category here
             category_id = database->get_integer("INSERT INTO categories (name, alt_names) VALUES ('?', 'Musical Instruments') RETURNING id;");
             // subcategories here
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = database->get_integer("INSERT INTO categories (name, alt_names) VALUES ('?', '') RETURNING id;"); // Sports & Outdoor Goods => Camping & Hiking, Garden (pots, plants, gardening tools), Cookouts & Outdoor Furniture (patios, etc.), Recreation (basketball hoop, etc.), Fishing & Hunting
             // subcategories here
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = database->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Everything Else', 'Miscellaneous') RETURNING id;");
             // subcategories here
             //database->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------             */
    //-------------------------
    // favorites (wishlists)
    if(!database->table_exists("favorites")) {
        //database->execute("CREATE TABLE ?tbl(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        //database->execute("ALTER TABLE ?tbl ADD COLUMN user_id INTEGER REFERENCES users(id);");
        //database->execute("ALTER TABLE ?tbl ADD COLUMN item_ids integer[];");
        //database->execute("ALTER TABLE ?tbl ADD COLUMN ?col ?datatype;");
    }
    //-------------------------
    database->execute("COMMIT;");
    //database->execute_params("UPDATE users SET verify_key = $1 WHERE id = 2;", { "myverifykey1" });
    //std::string verify_key = database->get_text_params("SELECT verify_key FROM users WHERE id = $1;", { "1" });
    //std::cout << "verify_key: " << verify_key << std::endl;
    //database->execute("INSERT INTO users(verify_key, encrypt_key) VALUES(\"myverifykey1\", \"myencryptkey1\");");
}
int main() {
    // Do some testing here ...
    start_and_initialize_database();
    // register an item
    Item ball("Ball", "A round and bouncy play-thing", 10, 0.5, std::make_tuple(0, 0, 0), "new", "0000-0000-0001");
    Item candy("Candy", "O' so sweet and dee-lish!", 2, 0.01, std::make_tuple(0, 0, 0), "", "0000-0000-0002");
    Item ring("Ring", "One ring to rule them all", 99, 0.5, std::make_tuple(0, 0, 0), "new", "0000-0000-0003");
    // register user
    if(!Validator::register_peer()) {
        neroshop::print("register_peer: failed", 1);
    }
    // create a user
    //User * user = Seller::on_login();//new Seller();
    //static_cast<Seller *>(user)->list_item(ball, 50, 8.50, "usd", 0.00, 0, 0, "", "new");
    // list an item
    // temporary testing code
    ////Encryptor::generate_key_pair();
    //-------------------------
    // Get private_key contents from file
    std::ifstream rfile (std::string(NEROSHOP_CONFIG_PATH + "/secret.key").c_str(), std::ios::binary);
    std::stringstream private_key;
    private_key << rfile.rdbuf(); // dump file contents
    rfile.close();
    std::cout << "secret.key contents: \n" << private_key.str() << std::endl;
    // Get SHA256sum of private_key contents
    std::string sha256sum;
    Validator::generate_sha256_hash(private_key.str(), sha256sum); // 1.7 kilobytes
    std::cout << "sha256sum of secret.key content: " << sha256sum << std::endl;
    //-------------------------
    //neroshop::Server server;
    //server.bind("exit", [](void) { ::system("exit"); });
    //-------------------------
    // Close database when app is terminated
    DB::SQLite3 * database = DB::SQLite3::get_singleton();
    database->close();
    return 0;
}
