#include "backend.hpp"
#include "../core/searcher.hpp"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>

#include <future>
#include <thread>

neroshop::Backend::Backend(QObject *parent) : QObject(parent) {}

QString neroshop::Backend::urlToLocalFile(const QUrl &url) const
{
    return url.toLocalFile();
}
//----------------------------------------------------------------
void neroshop::Backend::copyTextToClipboard(const QString& text) {
    QClipboard * clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
    std::cout << "Copied text to clipboard\n";
}
//----------------------------------------------------------------
//----------------------------------------------------------------
QStringList neroshop::Backend::getCurrencyList() const
{
    QStringList currency_list;
    for (const auto& [key, value] : neroshop::CurrencyMap) {
        currency_list << QString::fromStdString(key);
    }
    return currency_list;
}
//----------------------------------------------------------------
int neroshop::Backend::getCurrencyDecimals(const QString& currency) const {
    auto map_key = currency.toUpper().toStdString();
    // Check if key exists in std::map
    if(neroshop::CurrencyMap.count(map_key) > 0) {
        auto map_value = neroshop::CurrencyMap[map_key];
        int decimal_places = std::get<2>(map_value);
        return decimal_places;
    }
    return 2;
}
//----------------------------------------------------------------
QString neroshop::Backend::getCurrencySign(const QString& currency) const {
    return QString::fromStdString(neroshop::Converter::get_currency_sign(currency.toStdString()));
}
//----------------------------------------------------------------
bool neroshop::Backend::isSupportedCurrency(const QString& currency) const {
    return neroshop::Converter::is_supported_currency(currency.toStdString());
}
//----------------------------------------------------------------
//----------------------------------------------------------------
void neroshop::Backend::initializeDatabase() {
    db::Sqlite3 * database = neroshop::get_database();
    database->execute("BEGIN;");
     //-------------------------
    // Todo: Make monero_address the primary key and remove id. Also, replace all foreign key references from id to monero_address
    // table users
    if(!database->table_exists("users")) { 
        database->execute("CREATE TABLE users(name TEXT, monero_address TEXT NOT NULL PRIMARY KEY"//, UNIQUE"
        ");");
        database->execute("ALTER TABLE users ADD COLUMN public_key TEXT DEFAULT NULL;"); // encrypt_key - public_key used for encryption of messages
        database->execute("ALTER TABLE users ADD COLUMN avatar BLOB DEFAULT NULL;"); // encrypt_key - public_key used for encryption of messages
        
        // Notes: Display names are optional which means they can be an empty string but making the "name" column UNIQUE will not allow empty strings on multiple names
        ////database->execute("CREATE UNIQUE INDEX index_public_keys ON users (public_key);"); // This is commented out to allow multiple users to use the same public key, in the case of a user having two neroshop accounts?
    }    
    // products (represents both items and services)
    if(!database->table_exists("products")) {
        database->execute("CREATE TABLE products(uuid TEXT NOT NULL PRIMARY KEY);");
        database->execute("ALTER TABLE products ADD COLUMN name TEXT;");
        database->execute("ALTER TABLE products ADD COLUMN description TEXT;");
        //database->execute("ALTER TABLE products ADD COLUMN price REAL");// This should be the manufacturer's original price (won't be used though) // unit_price or price_per_unit
        database->execute("ALTER TABLE products ADD COLUMN weight REAL;"); // kg // TODO: add weight to attributes
        database->execute("ALTER TABLE products ADD COLUMN attributes TEXT;"); // attribute options format: "Color:Red,Green,Blue;Size:XS,S,M,L,XL"// Can be a number(e.g 16) or a text(l x w x h)
        database->execute("ALTER TABLE products ADD COLUMN code TEXT;"); // product_code can be either upc (universal product code) or a custom sku
        database->execute("ALTER TABLE products ADD COLUMN category_id INTEGER REFERENCES categories(id);");
        //database->execute("ALTER TABLE products ADD COLUMN subcategory_id INTEGER REFERENCES categories(id);");
        //database->execute("ALTER TABLE products ADD COLUMN ?col ?datatype;");
        //database->execute("CREATE UNIQUE INDEX ?index ON products (?col);");
        // the seller determines the final product price, the product condition and whether the product will have a discount or not
        // Note: UPC codes can be totally different for the different variations(color, etc.) of the same product
    }
    // listings
    if(!database->table_exists("listings")) {
        database->execute("CREATE TABLE listings(uuid TEXT NOT NULL PRIMARY KEY, "
        "product_id TEXT REFERENCES products(uuid) ON DELETE CASCADE, " // ON DELETE CASCADE keeps the parent table from being deleted until all child rows that references the parent are deleted first
        "seller_id TEXT REFERENCES users(monero_address)" // alternative names: "store_id"
        ");");
        database->execute("ALTER TABLE listings ADD COLUMN quantity INTEGER;"); // stock available
        database->execute("ALTER TABLE listings ADD COLUMN price REAL;"); // this is the final price of a product or list/sales price decided by the seller
        database->execute("ALTER TABLE listings ADD COLUMN currency TEXT;"); // the fiat currency the seller is selling the item in
        //database->execute("ALTER TABLE listings ADD COLUMN discount numeric(20,12);"); // alternative names: "seller_discount", or "discount_price"
        //database->execute("ALTER TABLE listings ADD COLUMN ?col ?datatype;"); // discount_times_can_use - number of times the discount can be used
        //database->execute("ALTER TABLE listings ADD COLUMN ?col ?datatype;"); // discounted_items_qty - number of items that the discount will apply to 
        //database->execute("ALTER TABLE listings ADD COLUMN ?col ?datatype;"); // discount_expiry -  date and time that the discount expires (will be in UTC format)
        //database->execute("ALTER TABLE listings ADD COLUMN ?col ?datatype;");        
        database->execute("ALTER TABLE listings ADD COLUMN condition TEXT;"); // item condition
        database->execute("ALTER TABLE listings ADD COLUMN location TEXT;");
        //database->execute("ALTER TABLE listings ADD COLUMN last_updated ?datatype;");
        database->execute("ALTER TABLE listings ADD COLUMN date TEXT DEFAULT CURRENT_TIMESTAMP;"); // date when first listed // will use ISO8601 string format as follows: YYYY-MM-DD HH:MM:SS.SSS
        //database->execute("");
        // For most recent listings: "SELECT * FROM listings ORDER BY date DESC;"
    }
    // cart
    if(!database->table_exists("cart")) {
        // local cart - for a single cart containing a list of product_ids
        // public cart - copied to all peers' databases
        database->execute("CREATE TABLE cart(uuid TEXT NOT NULL PRIMARY KEY, "
        "user_id TEXT REFERENCES users(monero_address) ON DELETE CASCADE"
        ");");
        // cart_items (public cart)
        database->execute("CREATE TABLE cart_item(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
        "cart_id TEXT REFERENCES cart(uuid) ON DELETE CASCADE"
        ");");
        database->execute("ALTER TABLE cart_item ADD COLUMN product_id TEXT REFERENCES products(uuid);");
        database->execute("ALTER TABLE cart_item ADD COLUMN quantity INTEGER;");
        //database->execute("ALTER TABLE cart_item ADD COLUMN item_price numeric;"); // sales_price will be used for the final pricing rather than the unit_price
        //database->execute("ALTER TABLE cart_item ADD COLUMN item_weight REAL;");//database->execute("CREATE TABLE cart_item(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, cart_id TEXT REFERENCES cart(id), product_id TEXT REFERENCES products(id), item_qty INTEGER, item_price NUMERIC, item_weight REAL);");
        database->execute("CREATE UNIQUE INDEX index_cart_item ON cart_item (cart_id, product_id);"); // cart_id and product_id duo MUST be unqiue for each row
    }
    // orders (purchase_orders)
    if(!database->table_exists("orders")) {
        database->execute("CREATE TABLE orders(uuid TEXT NOT NULL PRIMARY KEY);");//database->execute("ALTER TABLE orders ADD COLUMN ?col ?datatype;");
        database->execute("ALTER TABLE orders ADD COLUMN timestamp TEXT DEFAULT CURRENT_TIMESTAMP;"); // creation_date // to get UTC time: set to datetime('now');
        //database->execute("ALTER TABLE orders ADD COLUMN number TEXT;"); // uuid
        database->execute("ALTER TABLE orders ADD COLUMN status TEXT;");
        database->execute("ALTER TABLE orders ADD COLUMN user_id TEXT REFERENCES users(monero_address);"); // the user that placed the order
        //database->execute("ALTER TABLE orders ADD COLUMN weight REAL;"); // weight of all order items combined - not essential
        database->execute("ALTER TABLE orders ADD COLUMN subtotal numeric(20, 12);");
        database->execute("ALTER TABLE orders ADD COLUMN discount numeric(20, 12);");
        //database->execute("ALTER TABLE orders ADD COLUMN shipping_method TEXT;"); // comment this out
        database->execute("ALTER TABLE orders ADD COLUMN shipping_cost numeric(20, 12);");
        database->execute("ALTER TABLE orders ADD COLUMN total numeric(20, 12);");
        //database->execute("ALTER TABLE orders ADD COLUMN notes TEXT;"); // will contain sensative such as shipping address and tracking numbers that will be encrypted and can only be decrypted by the seller - this may not be necessary since buyer can contact seller privately
        //database->execute("ALTER TABLE orders ADD COLUMN order_data TEXT;"); // encrypted JSON
        // order_item
        database->execute("CREATE TABLE order_item(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
        "order_id TEXT REFERENCES orders(uuid) ON DELETE CASCADE, "
        "product_id TEXT REFERENCES products(uuid), "
        "seller_id TEXT REFERENCES users(monero_address), "
        "quantity INTEGER"
        ");");
        //database->execute("ALTER TABLE order_item ADD COLUMN item_price ?datatype;");
        //database->execute("ALTER TABLE order_item ADD COLUMN ?col ?datatype;");
    }
    // ratings - product_ratings, seller_ratings
    if(!database->table_exists("seller_ratings")) {
        database->execute("CREATE TABLE seller_ratings(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
        "seller_id TEXT REFERENCES users(monero_address) ON DELETE CASCADE, "
        "score INTEGER, "
        "user_id TEXT REFERENCES users(monero_address), "
        "comments TEXT, signature TEXT"
        ");");
    }
    if(!database->table_exists("product_ratings")) {
        database->execute("CREATE TABLE product_ratings(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
        "product_id TEXT REFERENCES products(uuid) ON DELETE CASCADE, "
        "stars INTEGER, "
        "user_id TEXT REFERENCES users(monero_address), "
        "comments TEXT, signature TEXT"
        ");");
    }
    // images
    if(!database->table_exists("images")) { // TODO: rename to product_images?
        database->execute("CREATE TABLE images(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
        "product_id TEXT REFERENCES products(uuid) ON DELETE CASCADE, "
        "name TEXT, data BLOB"
        ");");
    }    
    // favorites (wishlists)
    if(!database->table_exists("favorites")) {
        //database->execute("CREATE TABLE ?tbl(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        //database->execute("ALTER TABLE ?tbl ADD COLUMN user_id TEXT REFERENCES users(monero_address);");
        //database->execute("ALTER TABLE ?tbl ADD COLUMN product_ids integer[];");
        //database->execute("ALTER TABLE ?tbl ADD COLUMN ?col ?datatype;");
    }                    
    // categories, subcategories
    // Note: Products can fall under one category and multiple subcategories
    if(!database->table_exists("categories")) { // TODO: rename to product_categories?
        database->execute("CREATE TABLE categories(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        database->execute("ALTER TABLE categories ADD COLUMN name TEXT;");
        database->execute("ALTER TABLE categories ADD COLUMN description TEXT;"); // alternative names
        database->execute("ALTER TABLE categories ADD COLUMN thumbnail TEXT;");
    }    
    if(!database->table_exists("subcategories")) { // TODO: rename to product_subcategories?
        database->execute("CREATE TABLE subcategories(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);");
        database->execute("ALTER TABLE subcategories ADD COLUMN name TEXT;");
        database->execute("ALTER TABLE subcategories ADD COLUMN category_id INTEGER REFERENCES categories(id);");
        database->execute("ALTER TABLE subcategories ADD COLUMN description TEXT;");           
    // categories types
    int category_id = 0;
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Food & Beverages', 'Grocery', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAACXBIWXMAAAsTAAALEwEAmpwYAAACZUlEQVR4nO2aPWgUQRSABy0iokRIdWoTBJtoIVqEgD+NINHCZiWFprCykYDa2MUmimBjCrWyCAouBCwEC7GwUUyTwlILrSL4V0RBRf3CoMKRm73d2503vovvg+lm3sz72Jmdt3fOGYZhGIZhdAEYR47limvIQoML+uaBrnmVeYIAJ5FlQLuAKWEBLe0CpoUFjGgXMCssYL92AXeEBRzXLuChsIDT2gU8FxZwQbuAV8ICZrQL+Cgs4KZaAcB64KewgFyzgCHkeaxZwE75/FnULGBUPn/eaBYwLp8/nzULOEUaBrQKmCINLa0CLpGGEa0CrpOGjooQGP6TuG/XCsZlgfYs0O+e1kqwsCIEJojHrNZKsLAiBA4Tj4t1BSyQho6KENgbMf5YXQEvScNMwRkQg7fAuroCPpCGW4G5t0SKfb5W8h5/SyMNt90qgM0R4voneIOrC/CJNHR8EwAGG8ZcBnbXTt4DvCYNV90qgG0N4r0HDrqmAPOk4URg7kM1Yz0BdjRO3gOcQ55fwHYXvgj9qBjjC3AfOOZiArSAb8ICHnWZ358DY8CVgrFHgF3AxqiJtwPMCQs46kropRiKDrBV8Mvwg4pr+HcCPMCkQPLvvFzXDwI8/kYVMXl/v9jnKqJCgAc4C3xtmLz/pWmP6wE1Ajz+dlXw0aGM78ANYJPrEVUC/gIcAO4CSyXv+BfA5dC7vq8FtAOcKRAw5CLQDwIyyQWaAOwJyGwLCG6xxtgZgB2C2Zo+BPn9P4G8S3saWmDJmLyfBGQI0HR+2azbMAHYE5D971tgtOxAq9Oazi+btWEYhlvbrADFelwIpcui8wAAAABJRU5ErkJggg==') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Electronics', '', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABmJLR0QA/wD/AP+gvaeTAAACO0lEQVR4nO2bTU7DMBCF3yDgFGVBWSNY0ntQDsJBABXBHWgP0u6Q+FkCC3qKIvpYpEShHbeJk9gpmW8VWZN4ZuJxZmwHMNqNVPUgkh0AvZUOREaKbA9AZ6l5KiJjRbavdDcWkamvrrVAsk8Fh+xQER06ZDU0p3ixU9WDthVzQGwFYtN6BxiGYbSa0qkwyRMAfQCHAPZKa5SPLwAfAB5E5DlQn38huUvyjuTcka6GYE7yluRuDAfcRTR8mYGvHV4hwGTYP/reXwNzAKc+4eCbCZ6jOcYDiR3nPjemscMC9TyArk9nNXOkNXLDekJ28ugB0Gpy7U2Hmu2LsO9o12y6ADACrBgyB5gDYisQm3QSXMz2Tfq0VYKIrLXJRkDAvqYAVtb9HWj7BrUQ0gFjEbnII8hkj6Cytf91WAj8XpA8A3C5LJD3rTUV6jtOVyIyAf6GwAECDbvAaDaNAEwACwFzQOsdkJ0DPrEoEf8Zmk2fvxfZVHiCpE7+V2z6ilkIBOyr5/gmq7K1apIhpAM6aGCe0foQSB3AAoectgnHRko6Em0ExFYgNuaA2ArEJvsZHCN/JvhVgy5lmTnaNZvSpblsKjxF/lrgPb9ewXjTGh17mym+ITBCsiXdFObwLOS8HCAiTwDufe6tiVsReQnaI5MjMgOS31Uf9yjAN8kbljgiU8UhqWMkOX4X7i3qqpkhmYceROQ1UJ+GYRiGE9pPU9uJOSC2ArFpvQOqXBYvsp5wjdXqzfUr7Np63jDK8QNj5L+yJWBCUgAAAABJRU5ErkJggg==') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Home, Furniture & Appliances', 'Domestic Goods;Furniture;Home Appliances', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAACXBIWXMAAAsTAAALEwEAmpwYAAACoUlEQVR4nO2Yy2oUQRSGKwkqibeFF0SjuPC+EQQvuPJGEEEGldmKlxAUl6KQjQhufAEvoIIuTGBwo75BXAgRdRtcqJgH0BiiRoVPjvSQoadrpipOV1W39UHv6tR/6lD9V9VRKhKJRCKRSCQSmRdAH7APqABVR18l0exVvgC2AqPADP4Q7RFgi+vFDwKzhIPkct7V4s8SLkMutv0s4SK5bc6zAKOEz+M83X6G8JEc+/IowF6Kw548ClChOFTyKECV4lCNBeg0xB3ACf5zD+gGrgI/CZdvwAWgq+MFqAOcDLQIcv4fUS4AhgiPU51YWC9wEDgKbGgz9inh8LBNrpuAY8B+YHHWgEXATWAqNfEYsEMz6TbgN/6R33Fji9vry9T4L8ANYGGjuT1vIfAZ2KkRqOGfEU1uh9u8XWQHd8vAMwYiE8CCDJFD+OeA5lf+aBB7Wga/MBQ6lyHUBbzDHxNZR15yFJowJoOnDQe/0my1K/jjsianN4bxUzL4u4XgrgyxVcAP3COaKzPy2W0xx7QEvLUIuBtQp0hnfvcs5hiXgGsWAV+BJZrjpub4a2p6AMssfmlhWIL6Lc/zQRUowEWLdfwC1tYDn1kEjqtAAV5brONJY+Bx7GgyQ99Ymp8w0BjcY3hxqHNbBYal+b3/ewtsBLhuaYZLVSCIMSc5mTKcNUl/II+bvJkzvzRtHkVlYc780szDDIvIQKsC9FiaYdFoNr80lmZYNJrNT2OGITY9O/F4WqNMAO5TPm4ZLV4AVgCfKNe/v1zZAGwvSRE+SFfYavF1gNXAo4JekOTC8yCraWINsB64JA2R5C0+mSE46bAX0Er/TvIsXvfPC9ehaYfXlCN86yvfCfjWV74T8K2vfCfgW1/5TsC3fiQSiUQiEVUK/gB5u4ODDC6IPAAAAABJRU5ErkJggg==') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Patio & Garden', '', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABmJLR0QA/wD/AP+gvaeTAAAD20lEQVR4nO2aT6gVVRzHv7+UMk3RAjErynTRoiBCqAdtcmEiUiFkqxbRrnbiQiLquVKTstxIGOTGzQvJlegiwUUlRJYP+iMYQaWP0pSnkvi093Ex8+h6PWfuPTPnzNyb84EL78098/v9vt8798y5vzNSS0tLS0tLy+2KNV1AJ8AcSaslPSfpKUmPSrpX0lxJFyVNSBqXdEzSUTP7tqFS4wIsB3YDk4TxPfAGcHfTGkoBLAB2AtcChXfzK7C+aT1BACuBXyoK72YMmNe0tp4ALwH/RBY/w3HgwX5rqX0SBF6UtF/SrIJhVyV9Iem4pL8knZW0UNmkOJK/is4/KelZMzsXo+ZokF32RZ/8b8BrwPwecRYD7wDnC2J9zSBNjsB84JSn2GlgKzA3MOYS4ECBCe+n0hMMsMNT5BTwaoW4BmzzxL4OPB1TR9kiH86FunglUo6PPPG/jBG/anG7PMXtjphjNvCVJ88zsfKUKewu3JPVBJEnKeBJ4F9Hrn0x84QWtdbzqbyVKN/njlzngaLbZjqA9xwFTQGLEuV7wWP4Stf4O1IU0YUr8biZXUiU74ik647jzrtBHQYscxw7liqZmV2W9LPjraWu8XUY8Ijj2Juey7QMZ4DNXfH/cORc4iouyADgceAg8BPwIXBPyPmJuF/SVuDljmNTjnGzXSc7D7ogu5celrQgP/RY/lrTb4zErJX0Wf6367fERddJfV0BwIhuFj8TcEtAganp7AOscLzvnHR7XgG5+EO6VfzzZpZsMisLsEzSQ463TrjGFxrgES9JYwHiN/Q5riozE58v3zdB0YAHKP69PVqp3AQAd5L1B7tx3RZ7Btvcxy1ooEwANnnqDF92dxkwCawHLoea0IeJsTiBu9t0lYAeYWfhS8m+ApNkc4GAdz3JvSYkkRrGB8HiO4ofIRef/z8POO1J5DShNpluzgH3lTbAI+j1goTbBsiAabIOdFyAWcB4QeLRrvFNccuHEdOEVf0mr0Opg4+BtHsewKEeRYw2YMBMez39hg/wBFnbuYjR5JL/429gXXLhXSbsqVFgL8bKaKjaEHlb0qWKMRqlkgFm9qek8guNASBGS2y7pN8jxGmEygaY2RUNVmMkiFhN0U8lfRcpVq1EMcDMpiVtihGrbqK1xc3siLK+4VARe19go9y7MgNLVAPM7EdJe2PGTE2KnaGhWhxFN2DYFkep9ga3y70/N3AkMSBfHA1Ux9hHyt3hoVgcJTNgWBZHSZ8PyBdHEylzdHC6zEl1PCCxq4YcVyR9UubEWh6WBjYoe44gxQMVZyTtMbMfEsRuaWlp+X9zA8EbEeUfOYl4AAAAAElFTkSuQmCC') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Digital Goods', '', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAACXBIWXMAAAsTAAALEwEAmpwYAAADS0lEQVR4nO2au2sUURSHJ7Exia9SRURJRA2oENFNQLcUFXQ3ypYSNBZaqcFCUCSinfgfmMZkjaQSQTvFJ9qorYiFj8JnhEACRtFPDrmRzc25szNj2Mks94Ot5t7fnPkx9+ycc28QeDwej8fj8VQBKKEQMr4BKAIjwHtgEvgEPAT6gMUhc2WOzYhjrEYpSNMAYC3wjHC+AN11ZwCw0TxcFP4Ax+vGAKAFeEM8fgH5ejHgHMl4ITkj0wYwlfQ+OF7zIeAIcBEYcwTdmXUD2h3B9Fs6HcBPZdz5rBuwxxHMckXrsTJuIOsGlMKuW1qhD+gNwL8BI34JzMbngGCeJ8GCI5gWReuOMm4o60kw5whmRsEDLANGlXGXs25AEzCuDJESuBtYAmwB7juCPpBpAwTzyZuEb5VLJcsGtAE/EhhwwtLJpgECcCjmw98EGoN6MUAAeiK+CdeAhYFF5g0QgFZgUEmMv4EnwN7AwXw1YJUxYcYvwjz5d9gO7Ad2atWhMqdLuVeXY+ysmCTWpM/p8Xg8sTBJrtMUQ3lgxX9orTQaBVNbNEWYk04SBNYBZWBC6QI/BfbF0CqYnSSZW8mE+SttC5lbm7/BSoDDZq+vGmXtQ8d6e4Yj6MhHVc+8MICpr7w4SO2/QNFpBG7F1DqaqgFMvfZJip1TitbpBDpy79Y0DSiTjFGr3F0EfE+oNZiKAUCzkvCmGx5F0/DYDNxzBHWwWl0B3AU2Ga2i0bYZr/x3qKUBOcfNita4pabBEdbyuqJc/ypzLS3pJGlsS8OAwhw2PbWldFvRke12jUIaBpS0O6W99eUN0PFvQOCXwByDzwHsjnEAQnp+Nlcrrg8o1x85qkONXWnkgHbHzS5Y47aak19hR2D6letybKbD0pIzRRob0jCgwZz41LgO9AKXQg5B5aymp8aY0egNqRLfWifKamOAAJwlGc+toKUSfJlQ68y0ThoGNAOvYwYsy2GHopV3LJUwXtldopoaIMj6Az5HDFg2QY4FDuSYrNIFciGF0XpFo7YGCMAaR6a3A/73ze7CFDzVDJVjdasd82tvQEVSlN2eG8A70yL7aPb/T0rNH0REjs5L0wR4YIybNMluuFpvMTUDPB6Px+PxBFnnL5kjkrLcntRPAAAAAElFTkSuQmCC') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Services', 'Non-product services, Freelancing, etc.', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABmJLR0QA/wD/AP+gvaeTAAADjUlEQVR4nO2aS6hNURjH/+t61c2juCjKsy5loCiuiTKiFGUiTEQxo0gGCteEiYmZPGbUjQkhRhQDEiZInYG8out6XFeKe/kZ7H1qO/fsx7l7rb3Ocfdvck77nG+t7/+tb639rbW3VFJSUjKKMb4dkCRgk6T1kiqSThljfnh2qTiALfzLZd8+FQpwrSYAQ8Ak3345B5gHHAR6Gc5pYLVvH50ATAPOA4N1hNfyAOjy7XMugA6gM/y+AnifQXiUQeBAaD8DWOhXUQMAe4CfoZDnwECD4qM8BP6E368DE3zrS4Qg1X/lEJzGDht+ttloJIapksY5bH+GjUacBcAYU5F0N+PfK5KuSHog6XeG/w9Iav56AZgC3ElI4wFgZ41NJ/AkwaYfWO5LU8MANxudx8Bs4HOMzRAwpWgdIwZ4FyOkkmJ3PCFwq2z553IRrNIRc/15it2zhN+mj9CXYRQRgO8x19NEzEz4bWCEvhQPQQETN5c7Y2zagEcJU2CuLf+KyIC4VB8jqQeYE70ItEk6KWlZjN0XSR/suecQoDthFKt8DRe8bcD+lJGvch+Y6FtfIsDMMM1dsduGny6nQLuCNHfFZBuNuCyFX0q6HrlEziaj9p8k9eRsT5LjQ1GCLetWBbe0G5I2Sjo2gqbuStouaZ2kSZIuGmPe2PKzUIANZD8UGQROAON9+20VoB3Yy7+HHFHeAqeABb59dQ71d4vWSt00iiiE0qg9M3hhjPnoxRMfhNPhAsE+/yGw1LdPLQ2wBLgC9AGvgLNA3I70/wLoIiita3kKtNezsV4HhPf+TkmzZKFaM8Zcythvl6RbCX3uMsacyetPkgNrgMvkO/sfRsa+u8I1JInT9WzHWhA+S9I5BVVa4WQY+Sp9LjpfTlC0uOJnSv+rMow8BMXWStvis6RdXmLPDckuHuBQK4oH6LYg/miriq8Aw+Y1nsU30nkeHgHzixCfuQ4geBhxU/nv7b2S7qn+M8D3km5LumqM+ZOj/25jjL3RbzDyaSxy3H/Tp/3iZhKfOAWwl/ZRPirYAmd5DC5JazP239Rp75oj1oSPNvFxJ0K/lD1FfXLUGFO3WMoNQZ0f95JCM2A37VssCO7FR4LQbOvB4cLEN1kQXgObbWtzUQp/k7Qv/LTBkKQ3kh7XlseFQ/qa0E+rv9ycRkIQ/n/xVeoEYfSIrxIJwugTX4Xg7mDtpcWSkpKSkoL5C2IZ98cgmOE6AAAAAElFTkSuQmCC') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Books', '', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABmJLR0QA/wD/AP+gvaeTAAACCklEQVR4nO2bPWsUURhGz00iWlgYFAtbQRFUgiCI2NjZJgEbSZlOsNTKP2EnNmkCaZLCzkL8HYpg4QeSEAJiEVEeC7dY1M3c3XnnPpPde6plZ/flzNll7sDehcpskyZ5k6Ql4CpwHjgRahTPTkrp7aiDC7lTJCVgDXgKXAwQK8U7oF0ASaeADeB+kFRvmMt83Qum8OQhI4CkZeBBARcLOd+AJ51bGDkygKQLwM1CLhaavgGXmHCpPC40BVgsYmGkKcBUf/owxo1QBu+Bg8B5TSwA18hfykcOieAbcDml9CtoXhaStoHlNjNa1RvisPTJD/jedkBUgGNLDeAWcFMDBM1x3S+09o9aBs9Keg7sB83LYR6413ZI5I3QeuCsYtRrgFvATQ3gFnBTA7gF3NQAbgE3NYBbwE0N4BZwUwO4BdzUAG4BNzWAW8BNDeAWcFMDuAXc1ABuATc1gFvAzcwHaPPb4E/+7A3qOz8mfqekFf3LlqTbkuYDJfvJfwI8dDsV5a8AL90+XTDORXCzMwsjTQEOhx7vdSnioinAx6HH17sU6SWS5iR9HlwDPkg67XYqjqTHQxfCV5LOuJ0iadzeJukk8Aa4NXjqC/AMeA3sdqcWxteU0sg9xVn7+ySdA3aAO1FWBVlNKW2POpi1DKaU9oC7wCPgU5BYLxh7h+fgH6Q3gCtMwV9nK7POb7zRpspIHJDQAAAAAElFTkSuQmCC') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Movies & TV Shows', '', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABmJLR0QA/wD/AP+gvaeTAAAC7ElEQVR4nO2az2+MQRjHv09pohHclBIkUhc/Dhqk6iYSN0LqVxxxpPEPkHB0QURE4w/g5CROIkSJgyoJ4kaiKRJpKyG0/Ti8u0mzZrbv7nR3urvzOT7vPLPf7zM7M/vujJRINAyAAT2xdUShYP4GMAUci62nrswyX2QKOB5bVy6AbuBSQL4Bt/ifP8Ch+dQ67wAngYmC4DNV5PvMFxkBFtdCexBAB3C1ROwvYFsFfZR+7Ut5D3TV0kfVAIMe0W+Ajhz5c438O2BVPbxUBdAJjHrE35wjt3FHfjbAfmDGY8K5jTWN+SLAFY+RH8CGkrbNZV6SgHZgyGPoBdBeaNd85osAG4Fxj7HLTW2+CHDCY24aeFDG/MJe7SsBuFPGaLSRt5ziV0g6LemgpG5JK2spKoAxSR8l3Zd028zGg3sEjgLfKxy9hcA3oD/U/Dn8e3kjMAOcLefROwWAvZIeSloUVMX4TEvaZ2aPXA+dBQDaJA1L2lpDYfXktaTtZjZT+sBXgD2SnjgeTUq6JumtssrWg9XKFuAtgf30mdmzXC2Bi4759BfYESiiKoAlZG+SIVxw9d3m+cz1jthzM3s5f7byY2a/JQ0GdrPOFfQVYKkjNhooIJQvrqA5kHTP0XSZK99XgJYhFSC2gNi0fAEW3l/KFQLcdYR78+Y3fAEkBb3wtPwUSAWILSA2zbAGuH719Upamye54QtgZkdKY4WdIdfi2PJTIBUgtoDYtHwBGn4RBAjJb/lvQCpAbAGxSQWILSA2qQCxBcTGV4BJRyz2NZU1gfkTrqCvAJ8dsV3AzkARVUF2ufJUYDefXEHf4WifpKeORz8lXZc0ovodjnYpOxzdHNjPbjMbKg2WOx5/JSn33d4FzrCkHtfxuHMKFBoOqH6jXEumJQ24zEtldoHCjYrzkoJeNiKDMvOPq+8B+skuHDUaX4HDc/nLe01uubKF6ICkTZI6q65obRmT9EHZNblBM3NufYlEIpFIJBKS9A+F5c3zqqADtAAAAABJRU5ErkJggg==') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Music & Vinyl', 'Musical instruments', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABmJLR0QA/wD/AP+gvaeTAAAFHUlEQVR4nO2bXYhVVRTHfzuiJBuNMc2ERDJ0irEilF4sDaIPeopAjaCnihF6k0CUEqLGeQoVfEh7CsqH8CnNqBCCCgqJzI9yCMf8wjKFGY0+EH897Gtex3POPV/3XsH5w8DM3mevtf777LP2WmvvgQlMYAITuI4ROqVInQH0A/OB2UAvcCsgcB44AxwDhoG9IYQznbKtLVBvUJeqm9T9FsPFxpgN6hK1Yy+qMtRe9Q3114KkszCirlF7u80vFeoUdUgdq5H4eIypg+qUbvO9Aurz6sk2Eh+PE+qybvNG7VE/7CDx8diu3tYt8vPV4S6Sv4RD6txOk1+o/tZl4s04pT5Uhkvh7UVdBOwm7uHXEsaAx0II3xcZVGgC1HuAr4A7iozrIE4Di0MIw3kH3JD3QbUH2En3ye8EhlL6pgMfN2ytF+q2bn7kDexQb27YM6BeSHluW93kV3SMYjr+J99k10DG8/XECcYI70RHKKbjKvJN9q1PGXPCHBFjHh+wBphVdOJqRi8wdXxjY1IWpIyZBayupNWY2LQzti+CEbWvmbxxZWRhzBYJ1I0t5uBVoG6POgrsAg4AxxttdwEPAE9k6JsD7FIfbsjYDjzTQlcPsBJ4u7CVxnz+SI1v8LD6gnpThs6p6ir1aIacb2z95psxYpl6grGYURc2m0E8Qfdk9d0a9S8pMwGbalK+qrDyyzasrcmGDWWUFy1jJWFzWfJNdmypwY79afITvw1jAfNUWn9OjAB9IYR/K8hAnQz8RHSUpcUA05MKrWlxQD/VK8avVyUPEEL4E9hYVQxwf1JH2gTMr6hwFPioooxmvAecqyijL6kxbQLmVFT2aau3ry5QNzZ++rOeDSGMAp9VtGl2UmNaIFS16rovq1NdAHwHTGo0vaIuCiGkOivgB+C5CjZdFUpD+gqoGv2dbNH/MpfJ0/j9pYoyWyGRU+6CSEG0cqDmbCsisxTSJmCsotxW2eNW4O+mv/8iOros3FnJonj+eBXSfMBoRWUPZnWGEPYbi6uXlv3WEMKBFjITt7ECyM9JXVkx8hpTE51OGRhT36pp+UCS7LRP4OeKNvfQ2qkVwXKqO+ZDSY1pofA0Yom5iuM5BtzbiORKQ51END5xH88rBpgZQvh9fEfiCmjEzAcrKIQYu79TUQbAJqqRBziYRB6yt8EvKiqFGOCsLTtYXU2MGapidxnlSyo6nWZsMWZ1eXVPUrfWqH9pmQkIxnJSXuwwlqvScNRY7krdHYze/kXrvV1yRE1d6ZlOzrh838oxXzuJcfpU4Fuyk6lzxMRmL9FRBqK/6Aeeov4i7LoQwpulRpqvLH7FoYXaZ7GV0060LIvnmYTBDAWJJzbqDLM/h05hsBW/PMnQEOmZ2L4Qwj8J7aPA2Ryy24mTwPpaJKnLMmZ5YNyzeU5sOoEVtZBvIpZ2PH7BxiR47ZDPfTyeO9Q1XjrYA8xLeWSIeFDZ6riq3RgGFoYQctUQi16RuRv4GphZwrBO4A/gkRBC7mSuUEUohHCY+IarFkzagXPA00XIQ4mSWOMW1mKq1+jqxFngyRDCno5pVOcaLyl2G4fUNL/U9knoUT/oIvn31e7fVzTGCZ28R3RcXd5t3lfAeJlq0PZeqTmvrlNv6TbfVBgTqDXWmxD9or6m3l63vW37VxTjtZRHgWeBx4H7CugT+BH4HPgE+DKEcLEddnbyn6amES9CzSPm/9O4fOH6PHErO0qM5PaFEE53yrYJTGACE7hu8R+yRA2BiW4OUQAAAABJRU5ErkJggg==') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Apparel', 'Clothing, Shoes and Accessories; Jewelry and Watches; Fashion', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADwAAAA8CAYAAAA6/NlyAAAACXBIWXMAAAsTAAALEwEAmpwYAAABhklEQVR4nO3av0rDUBSA8dBF7DP4B1wERxF8kD6HQlEonZrRVxDBTaFdXH0NDQ5uuluJ0k30kwsJ1JDatMk9p17Pbyq0cO7HTW5LSRQZY4wx5s8BtoFj4BZ4BCb4M8lmuFlHwJZk6CZwDnyg5xMYATu+Yzued3JRbi0dX7F94IvV49bUbzo2XnIxD8D6AnPWgLslZ51px+YugFaFOS3gknpi7djcFdD+ZU4buKYZsXZs7gk4AfbcZZ5FutenwDPNirVjNVS7pwOJrRZNWLG58migS7i6ZcFjwjUuC04IV1IWfACkhCd1bbPu4/3ALu0UOJx3UleJHqCvVzu2YvQg+4yqbA292rFzogdT76uaWkevdmzhIEuy8B/fZSgr+Q3xCtzPPKDqWqVgEViwLNth31DmPbDIgoVF0lBmwb5hOyzL+44WCfdZcCQN22FZtsO+ocx7YJEFC4uk8Q+D3xV7U43gG8XgoUbwbvbXqLQXYEM8eOqpPPdE3JtAqJsxVIs1xhhjTFTXN9+cDWs9QZG1AAAAAElFTkSuQmCC') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Pets', 'Domesticated Animals', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABmJLR0QA/wD/AP+gvaeTAAAEcUlEQVR4nO2a3YsVZRzHv4/HsjfbLLctaMNqgzapC7Oydwgh0kuTqLvQiqD6A4reichujW6KbmqXrbQwNuyii7rpwlXXtDKLCs1Aio1W18yV/XQxM3Ac5zzzO8955pyF5gPD4Tzzez9nnteRampqampqamoiAThgAzAJnAD2AI8BC9qwsQB4PNU9AXwDbARclbFHAdhEMe9ZigA0gJEWNjZ1I4dggHtaBJ7xjMHGcx79OeDubuQSBDBeUoDjwBUe/UHgnxIbn3YzJzPAecBsSfAAL3hsvGTQPwmc283cTAC3GoIH+M5j4wejjZWx4jb3zAYuNcpdB1ycb0zbrjXa6DdHVULMAlhtOUnDBe3D6T0LC41ypcQswJ9tyJ7xD5C0tA39qTZkvcQswPeSMMqeKmibM+oiqWU/0i7RCuCcm5J0wCj+R0HbEaPufufcX0bZUmL+AyRpzCBzUtK+gvZ9kmYj+egNJBOZ4yVD2DaPfkcTqUpJk3seGAXeAG5pIfesJ4E54C6Pj6CpNLAqjWkkjXEwVt6ZgzXA0YKA3gHOzsk2SBY+Rbxm8GVeTAGLgHcLZKeB+2MlP9gi+YwPyS1TSZazGzl9SfyI0V+2nM6WwrvT7/nkHbDVE9dRYjwunl+kmQ0dO2o/rkcNcb0ew9G3BkeHgGizM0NMC4HfDHHtjeGsrFfPWN2m3QHgvvQaaFN3tTGmY2W2LPMA6wztTosQsAz4SNJhSdvT6zDwAXCl0Zd1U6Q0dksBDhqdlQ49wB2SJiStk9RoutWQtF7SBHBbDF8ppbFbCvCl0ZnXFnCDpHFJl3jE+iV9BizvxFcTX5UJWAy9Jdsi5+dWN0h2cMYk9RnsXCRpDDgnxFezWyWxdw6wzdDh3OTRf9nYaTXzosfeSoP+1ijJpw6XAj95nG336F4OzAQU4Bie0QH43KN7APA9akFFGCI5oMizC2i5RQW8GpB8xiseu/0ks8Q8k8A11rzaOmkhmew8IOnetOlrSSPOuX9byPcpeV6LdoAsTEm62jn3dwv7iyQ9LCkbOb6QtMU5V7Th0n0Ie/bztOwL5jUkz/50hAJMA5dVFWfsHSFJEnCWpFFJiyOYWyxpC7ll97wGeDPCL59nc6/zKoVkM2RzBclnvE0XV51tAfQBn1SYfMbHJKPL/AFYCxzsQvIZvwPrep23gCXA+11MPM8IsKRXyS8n2QnqNYeA60PzCHrnBrhA0m5JQ6GOI/OjpBXOudIdoDyh84CHNH+Sl5Jj9QdDFEMLsCpQr0osO0lnEFqARrlI1wmKKbQAewL1qmQyRCm0ExxQ0vHEmOvHYFrSkHOu6NjdS9A/wDl3RNLTIboV8VRI8h0DPInt1biqmAWe6HriuSLcDkz0IPkd2M4QqofkpHY9yf5g1exMfUV5cTr629fAsJJ9ujWSblTnr7SdUjLqjEsadc7t79DeaVT6+jnJlPlmSSskXZVeyySdL+nC9FOSZpT05DOSfpH0a/q5U9IO59xMlXHW1NTU1NTU/D/5D8Q7H3ts84ozAAAAAElFTkSuQmCC') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Toys & Games', '', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABmJLR0QA/wD/AP+gvaeTAAAEAklEQVR4nO2aS2hdRRyHv0lrsb7ahQk2ol1119YWQaUgCq31QYUq1ErBR0QFBSm4EcWF4EY3KiotIhUfuyAUBB/YjeJCSx+SpO1CUYwLH6W2xi5iG5PPxbmhye3MfZx7bk6Se7/NJTP/md9/frlnZu6ZgS5dunTpYELehuo64PHKn++GEI7ViH0Q2AL8DLwVQjibV3deoK5Tx73AuLo2EfuCszmoLp3rnAtFfdOLeSMR+1skdtNc55yiJ2e7ayJluyMDFVgVib02p27h5DVg0ZDXgKkWdSdbbF8YeQ0YbkFTYKSF9uWjXlmZzZtlSn2p7Pxn0so+YClwM9BfKboFeDYS+kDlcxIYCSH8mFdzXqPuSKwC85qOXwUaegTUFcATwHZgDdCXU+8U8BPwDbAf+DaEML+/JepO9VSOCa8RflCfUpeXPc4o6u7KzN1uflHvK3u8s1A3q//NweBn8rm6puyxo/aow3M8+GnOqtvLNuDWkgY/zZT6ipp7n9IoqWVwc7uF6xCA54B96pJ2CqUMWN1O0SYYAPbbxlUiZcDl7RLMwb3AZ+pV7eh8oewEbwcOqLGXKy2xUAwAuAkYUrcV2elCMgCgF/jE7J1kbxEdLjQDIFshngFG1b3qFvWyVjq7CHUQ2JG30xI4DxwGvgY+DiEcbbThQvwGxFgGbAKeB46oX6k3NtJwsRhQzW3AQfXFervJxWoAwBLgZeDtWkGL2YBpnlafTFV2ggEAr6pXxCo6xYCVwP2xik4xAGBjrLCTDOiPFXaSAdHlsJMMiNI1oOwEyqZrQNkJlE3HG5DnutoYsAc4RnZZahfQ0E/PHAwDHwB/AGvJDmivbpPWBdTBxIHFiHpdVWwwfm2uVfaol1Rp9VVyyMNgqwZMqDck4pepQzkTizFk1eBnaG0035ll1IBm5oBDIYShWEUI4TzZV7Uo3g8hTCS0vid7/VUIzRjwd536M60k0mRfp4sSSk2C/0TK1qs9IYTUHcHY4zEGfFknh63AiqqyDalgtQdYH6k6DpyoofNdnTxmiexMPEePJuL71bFGn7uqtrH5ZszEKZA6kMituLfYaq/6b0RkXH248l+Yjl2vnkgkNdCA1mOJtsfNruRPx/Woj9TIq9jlUX0vkZjqqPqpelSdTMT8qV7agM5y9WSij0n1SEXr1xr57Ct08JXEVql/1RCtx64mtB5qQeeM2p7b5+o96rkcSe3NofVODp1z6t3tGPvMxLZVXG6EKfU1Z8wRTej0qK/b+K200xZ8UlwrudXqR9begR1Stxagdad6uIbOhPqhen0RY2vqEpLZ0nQX2TrcR7Y5GgUOVHZohWB2nLUBuIPsus5K4CTZj6MvQgi/F6XVpUuXLh3N/4LZQfQlM+IfAAAAAElFTkSuQmCC') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Baby', '', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABmJLR0QA/wD/AP+gvaeTAAAD9UlEQVR4nO2bS4gVRxSG//IxzvhYZaNGxpnJJghCBBdCdi4iIRsNqLjQvYgEAllkJbjwAQkJqItsgskQJIIJYjDim8EgQUEQjeAwhBkFRUVdRGKI5nNR3Vpz0923H9WPq/0t+9Y9db7TVX2rq/tKLS0tLW8wpu4EfAEYSR9J+kDSW5ImJB0yxtyoNbEqABYBv/F/ngG7gRl151gawBJgPELeZXfdeeYG2ARMAA+APcAc57PBFPLhSHi3To9cAJ8A/3XIXAc+B/YCj1LIh+yI6qPWiyD2wrVC0rCkxZKQdEfSJUnrJH0lfzmOGmO2dB6c5Sl4JoBhSZ9JWitpUUXdPoo6WGkBgH5JuyRtk9RXZd+STlTc33SAxcDvGeasT8aC6Vab/AgwWZP8OLCwTvmlwJ81yd8EliTlV+qwAEYknZM0WGY/MTyUtMIYM5XUqLQlYs3yknSgm7xUUgEaIC9Jd9M08l4A7G983fKStJkUN0FeCxDIn1f98pK0StLWbo28FaBh8iGruzXwUoAGDftOrnVrUPhn0JFfWjSWZ65Ket8Y81dSo0IjABiUdEbNk78haU03eanACACGZOd80+SfSBo2xtxP0zjXCAjO/Fk1T16S5knqT9s4cwGCMz8mu4nRVFambZipAIF8VRe8xwW+m3qTJXUBHPmhPBll5EdJI5Iu5Py+3xUudvd1oqJb2J+B2UG/c4FTOWJ87FN+iOru548BfR39DwC/Zozzni/5KjczbgEDMXnMAY6mjHMbH9tfQD9wpSTZOL4h5g4OmA0cThFjf2H5oMOdparG8x0wMyanmcBownefApl+oeKqvUDS9hx1y8pPkkY7jm2R9APBhdDFGPNc0r2EePuMMZOFswLWlX+iOYId1jOAgxGf/4J9juDmtSsh3kWc54ZFC7DHu+50juCcYezQ/j6i3XGCCyPwRUK8ScDfEybgW+/Kr5gmn6IIp4GvE+JNYfcg/QHs9+/9kjFgfky/cdMhjingHa/yQSKflmHucAaYG9N33EjoxP+Zd5JY5ln4n4hj3YrwRy3yThJ51uBR3ASGsWv8TiKnA8lX+3KGfUQSy4EnBeXHgbeDeH1EL2enjQTgy9rlnWQ2YN+vKSTvxEssQqPknaQ35yjCU+xucVS8uCLcapy8k/RG4N+MRThOzMoMuwKMuiY0T95Juo4iNEM+hHzT4SgdmxxOvG4rvObIh+BpJGBfYe0t+ZCiRehp+RDyT4feG/ZxkG8kvB7yIZ6K0JvyIQWL0NvyITmL8HrIhwDrSX8DdZmY5XJPg72LPJkgfh/7vn/VL0tHUtqboth/aHwo+xh9QPZ/ABclnTfG/F1Wvy0tLS0tGXgBtI9sAulPU94AAAAASUVORK5CYII=') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Arts, Crafts, Sewing & Party Supplies', 'DIY & Handmade', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAACXBIWXMAAAsTAAALEwEAmpwYAAADZ0lEQVR4nO2aSWgUQRSGyxh3AwETxA2CuIBIiIoyQXDBCCGgKCQXRTEXEdccFA/iIXqIS+LFgzuCKIpiwIsQEAUJKBrNQcQFBBUVRUUFFfdPKtOEztg10z1d1VNt+oM6TA31/qrX1bW810IkJCQkJAQDKAM2ASedshEYJSICGAZUOaU4Kt0egAbgE//yEagXhgFWA+9cuq+A+aZ1ewAWA79QI/9bJAwBLAf+eOi+Nz4DgQHAA3Jz35D+QOCpS+c58N31e4MJ3V6ASvwzXWgGqHXZb3Pqql11R3Vr9gFYGsABS4RmgL0u++udunJX3XHdmn0AFgRwwFyhGeC6y75cBA8At111W3Vr9gEYAXz2MfjXwGChEWAQ8DWL5k9ggk5NT4DdPhzQJDQDzMmheUG3ZrYncTFHZ5qFZoAtOTTn6dbMtR2uAe4o9mS5VRUJjQDnsgy+W6dWIIBTik7VaNaRe76KRp1agZBHUEWnzmjUGJtl8G/lvUCXVr6vwyOPjskVu1STRn0WB7To0AgFsF3RuXWa7O/Pcu+o0KERCmA08MOjg7c02b9W0K3PD0C7opOVIe0WKa7eRk6beQPUKTrZGtLuNIXdLmETpJ/UM4+OvpGHp5DBDy9WCtsAmhWdXRbC5kEPey913zW0IC8jimjRpRA2b3rY2yFsBehQ3NTGaLoByt9lwlZIB0y92JaHrRkedsxGfcIi301n4cvkYR621ureViMBaFXMglRAO8cy2neIOABMVVyTjwS0053Rvk7EBaDTwwHyRDfcZ/uhGcfrx7pjDEYBGhWvwSqf7d3h7t4ocGwgnbv74OGAqz7bb3a1kXZGirgBHHKm8V3ghBPXW+izbRPwxFlL9ok4ApSEPbLKJ+933UiwGaDCiR+ukBkc4DBw3imnnd8tMsHp5ACnyJ1AxBnSHy/IRMo98kO+/13ALmB2LLZB0uGxnT7T6EGRKbc2YKKwDWC8zM4C3zDPbycEN9mGgRc7T/wL0SM/jtgDDClkAKSTwnMDGBf14CcpYoCF4kVkOQLSn8jZNHj3hak8Cgdcxl7Omh58LfZTY9IBV7CfdlODL1XkAG1DnkVKTDigmvhQZcIBs1wXGNvLTO0OSOjvACkLprbfkooy7WUjDYkDdEMyA+j3r0CqXy+CCQkJCeI/5C+Xr1Nj9eUFLAAAAABJRU5ErkJggg==') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Stationery & Office Supplies', '', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABmJLR0QA/wD/AP+gvaeTAAACvklEQVR4nO2bPWtUQRiFn1dFiwiCKPiJX72g1a6ton/AGK0sBEvBRrEVLVXQSv+AmgSLWAqmTCqRpDCCQSRisAhiuoh6LHYDIe7d+biT3N2985R33znzzmHncHf2XshkMpkaY6mEJF0EXpvZSgKtvcAV4BQwFDD0O/AWmDCzP2X78EbSBUm/JT1LoDUiaVnleC/peIq1+TR8QtLSmsmvl9BaNTIF85J2pVxrp4aHJM2sm3hF0pkILZM0l2jxq9zdiHWvbfhlwcTfJB0I1DuZePGS9ME175Z4C7gFXCr4bD8wLml7gN6xEr0U4cyBKAMknQPuO8qaQEgohpiVTDPYAElHgefAVo/yqyoRipvBtpBiSUPABLAnYNgTSbNmNhXUWXceAtMJ9dy0Q+9FZBgtSjro0B8O0BtOta6QLXAbGImcZx+tUNwROX7D8DKgHXr3Ss7VAJ6W1EiO04DA0HPRc6HoE4JFofcRmOkybjdwtsP1x5KmzazbWBcNSV6FZjZWYh4oCKElSUcc40zSK98QCwxBb1zri70TvGNmX7oVmJmAa5H6m0asAT98iszMq65KyvwWGAiyAVU3kJAx64Br0CAZEEU2oOoGqqb2BgSdB/Q4TUmj6y+aWdGxHTBYBhwCgs8Jar8FsgFVN1A1/ZoBU8DXFEL9asCj0gcdbWq/BbIBVTdQNbU3oF9DcNTjvBMA15lA7b8B2YCqG6ia2hvQryHYiWlazw0EMUgGLMTcHtd+C2QDqm6gagYpAw53+tvdlQuDZEAD+O9UGMcT8XkLRI672enrVpJfifW8NGMNaEaO68bnDdCcdxX00haYBeYSazpvjHrGgPYzRTeAVK+6fAIeuIp6xgAAM3tD612hnyWl3gHnzWzZVeiTAUmOn9exUPSBmY1JmgQuA6eBnQG6i8AkrZem/pZrMZPJZGrAP+s7Gd91ZI+yAAAAAElFTkSuQmCC') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Tools & Home Improvement', '', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAACXBIWXMAAAsTAAALEwEAmpwYAAABXUlEQVR4nO3ZS0oDQRSF4WtwB451Az424QIEwQX4GOoiIg5EJ0EXYISMegO6EjfgRB05iRPRX4q0EIhCNanursf5oCcZ1T251VT1NRMRERGZAwyAa+CN+Lk1Xrk1WyjACek5ChnAPekZhwygIj2VAgiFxDsAOAUugO1SA6jq376BW2C11AB+3ZQewFej7UB+ATjD0gOYlB5ApQBKOQoToAOOSc9hyAAG9RXzlfi9AJfz1+GlA0idAkAdUGkLLNI7wGIBrAFnwF29X0M/z0sdhdsE7APvdG8YQ/G7wGcPxbvr8Fbfxa8AT/Rj1GvxDrDT0z8/avxJrA3A3j+LbOMlOAHOgU2LBXDwV/VWChQA6gC0BRZZ7oAN4BH4oKv5fkyYFd/tfD8mwLTz+X5M8BfPdTUkBeBPHWA5Qh3gTR1gOUId4E0dYDlCR2Eems73swKsMwth6jvfFxEREbEO/ADiFbekRtGUMAAAAABJRU5ErkJggg==') RETURNING id;"); // Hardware - Heating, cooling, flooring, paint, etc.
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Motor Vehicles, Automotive Parts & Accessories', 'Auto, Tires & Industrial', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAACXBIWXMAAAsTAAALEwEAmpwYAAAFjElEQVR4nO2ba4hVVRSAj1ia2mvGLMseakX0I7LQ+tGP/gVSSY9JA6UyzXLCAqnwkRmlZkUQURYJzWgjRlYaCSVB6WQhlNjLmXLKXhBlTS9LM535YjHr3k7Hvc/ed99z7r0D94P7a52z7tr77r32WmuvG0V16tSpUyc7gOHAZOBRYBXwItAGzALGAsOAa4AHgBaVrwGagTOBo4GrgOX6fivwEDBJZFGtQp/xzwN/8x8HgCeB04BTgKeBfTH5IeA54AyduMeBP7Dzuz4zPKoVgAHA3cD+hLGfARfoM7cAfyXk3wIXq/xqoBt/fgaaqj32CBgMvGQw8HVd6ufp8k7yDjBCdSwmjF7ZRtUc/CDgNYNhHwELgQ8thn8AHKs6llI+y6s1Ac8EGNsDnKvvi1PMitsqPfgbyjB2O3CpwWeUgzjbCZUa/MnAL2UaLPs3az4Vn1SJCVgdYNw/Ae98DszTzy7PdxbkPfhxuo9N/Am8a5FdCWwtYfDr4kGPBkem0ybJb8AJeU7AOsOXfgPcDBwDrDTIt+m7DcBuj0FslhPGcups8Xh/WV6DH62RW5LZKh9hcWxTVC5xwa8eAY71FwRO9AiYRD4sjwl40PBle+VM1zB3vUEuzvIofX8mGRxnmjO4mJlHuLvb8EWS5CzS/W+iLabD5QM6gIEetgzUZ9Noz3oCJgR694Pyq+oKsTnP/20lT3uaPY7ZU7OcgEWUxw4P7+2d6qrDlcwwjVuznIB28qUlwCapFaSxIavBH5nI4U1haLnR3uQAu6Y4dP4kviuLCRhv+YIejQqvsMibNO53IUdrY4BdDepj0jg7iwmYbvnVC1ndVMughupe3egw8v0ybJPUOtOVdRhaj0uyKyZfYpDvjMnnZL3/Y7qlXpjGfaG6i1hi8E0OI4oOSGt4adwVBQLc49C9JlR3EUuC82xM/oZB/kRMbqoaxZkYBQJc7tC9OVR3EaDLoPhhlQ2xyBfG3t/mMHJcFIgUXR26O0N1F7EkHwuASzRnN7Ffg6dBKc8UGBuVV4pPY0+o7iKGcnah+uIKbdGYXRKmNILzd81A09gbqruIJQXOksNy/xLL8mkcDNVdpD4BGLeAa4l/VcIKqPkt0O1wcl8Y5PP0MvN7jwmoeSfY5TjmttvqchLjA1/392NwqyPQedMgXxGTb6piIPR2XpXgDTG5XG8n2VhCvJ5nKNyWVzK0MyY33e5+4pBnlQytqkQyND0l3W2wVIukSeJ+YCRwXX9Phy+0KJ+qTRBpHNCLjjSkqNEQYFejR5B2Vp4lsV6yI6Qkdr1D555MSmIVKoq2Rtlf0q4vVacV4F7Hl31Z5gTkURaf5auvnMJor0aEp1u2hJwAr3pmjs0l2HO7Q5fYMspXn+/VmOlXXuuoHK2MhawfO4zuLOFqTJ7NtxKURBsbTZefQ1R+p0Euy3RoCW01szO6HJ0RZQ19zYymOvyNKh9puR7v0PtBudr+wWF4d6F9zmLDSR7tOflcjwva1prkvZh8RYph+/RocrHF0iAx2PM0WhLlBXC+xaFdFlslIf1ASV4WT5/w+q94nib5ttFiPn+lRnhExjGDpOHzgTssKbmJ+bkOPrbXTUWSOY4tkDcdhW6U3AGmBYbGO1Juk8tBkq/xFRl8AeApD6OSSL/fOVID8EikSiG7qK/EJEmiPBOd2lUmPURJ3lKPPsAjxPZhaVQt6CuKvmA5yho1ajMVLdoL3hqYG5hZyjuLqzb4AvpLzjWkzLLcL9JnZhiSl67YHyomAt+VMHjp/rg2qiXoW/ItiWjwkPqK0ZowtSR8Q4/WFEcBx8mlK/Cj45x/LKSjpGLQt/SbdDCtGj2u1UbJMcDx2t/ziMYUhT9N3SStbbqtJumfplarjmXab1y7f5qqU6dOnagf8i84W5AagP7l2QAAAABJRU5ErkJggg==') RETURNING id;");//category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Motor Vehicles, Automotive Parts & Accessories', 'Auto, Tires & Industrial', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABmJLR0QA/wD/AP+gvaeTAAAA1UlEQVRIie2UvQ4BURCF7yxRIqIiosJjewLvoFlBI5EoEI+gFrF8milurp87N0gknGoz+51zZopd5/76agENYAjsea4TkAO9lPAmsIwEhxqnbL5Q0wroRviqsgdLeB2Yq2ENtAyeTPkiBtaAmcIbS7i5QM+cKrgF2pZw9YqlYKTQDuhYwz0/wDmciwccnXOV1OBAFxEp+YPMe341/K6yOPKayimwiEg4A/CR8P13XRBsa9LHL/ALnn/mNt1k+AWzNxQ8zgAGwAQoEn/RqCcH+m9Y8td0BU99R2bmnxZMAAAAAElFTkSuQmCC') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Beauty & Personal Care', 'Cosmetics;Health, Beauty & Personal Care;Hygiene', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAACXBIWXMAAAsTAAALEwEAmpwYAAACPUlEQVR4nO2bP2sUQRiHXzQQ0ZDUgiKSKkQFEVSMprFKUBCiXyB+A8FCFIxKUgf/1CpcCDlioZV2iaCVtYX4AQxaREVBk/jIkCHIMXuJzO7M7M77wFbHzu+d52bn7t7bFVEURVH8AU4Dbc/jAXAJ2CV1A7hCebwDDknGAgwfgH7JWIBhSjIX8F4asAn68EvqDp5I3UEF+CF1BxXgh9QdVIAfUkeAfcBRYAx/xuxYeyV1gIvAC+An5WPGfA5ckNQABoHXhGMJOCwpAIwCXwjPZ+Bs7MkPAavE4xtwJNbk9wAfiY/pG/TGEHC9oKAN4AlwxnwalPSJMgI8Bf4UZF4rZ1Y7BNgNrDgKWQMmpCKAyzajk09B+4jAuYJ34laA7NsF2SNVZ28B3HQU8KOMJb8dQF/B94wbVWdvATxyFLAkgQCWHfkPJWABC44CFgPmLzryFyRgAW1HAe1c8iV2AbHzpYRubxWoANEVEAj0EkD3AMrF/MiZBybtMQes57IJ/gbGHRnn7WuNFzDVJeduDgIOdMk5mIOA3m26To0XcLxLzokcBLS75DzLQYDhMTDwz/gDtq9IUwTMA1eBadvKdmHa6y+BV8DX/xw/aQHTHeeeLGhs+pKsgP2O89/kLuBtTgJmOs49ldslgG2imk1wBvhONSQtIAQqQAKugDnSoxVSwD3S405IAcfs3+CpsBH8Rgk2H29JhdmgkzcAPcD9yCvBZM+aWiQWbN7DZ1pXrRIemtrp0bKZw9EmriiKIg3gL31Q7N9UOef0AAAAAElFTkSuQmCC') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Drugs & Medications', '', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAACXBIWXMAAAsTAAALEwEAmpwYAAAC6ElEQVR4nO3by4sUVxTH8Ts+QAUFYYIzEtC9joriRrJOVq4kW0MIGh0JulCI+FgmvhbR1WShZi36B6gkoKIEdOljkWXMBDOObrJQfHzkMtWh7ame7sxUVVd31Xdf99zfPefcx7m3QqipqampKR8YwS4cwmlcxHkcxVfYgKEwSGAF9uN33fEnzmB96GewGPvwj/nxBhMYDv0GPsUt2fAMn4d+wUwexzDOkrcYD2UHWzHdhaCnuINruIFHeNfFd+P9LP5fnMLmNt+vwV780SESvgh9KP5KFNhlW0txEK/nmBOG+0X8exyfz9qOzzDVpt2JUAawBc/nEP/dAtvfgVdtlsh12SnJx/MHMrIT0yGNH7Jov5Seb7G1BE9S7DzMykYpPd9ic08be+sHXnzTISptn7A7DLr4BnicYvf7MGg53w7cTLH9Uxh0zzdIts2tXAqD7vkGuJvShx9DD7e3R3IzPrsvQ5hM6cPBXp7q7mFVLh1Ij8Q0dvbySFvYICT1w1biFnllkTnfjgdYnWlHPu7TaHKUbuV6Lz1fSCQkuX9VOnvLIj63QcAJ6fyNZb0M+9zTIakOxaU2jW/K5PnMIyFuruYQ/ysWZSF+LAfxCx6EDuJjtXk0q7r9XzmJn/cgdBD/AhuzurG5nbP4/z0IXYjftmDxkVhbVywdJ8YOE95LbA8ZXlROKZ62kVCY55uM9YpZg1Co+Aju6y3/pUNhYd8Aa5WDGAlHCvV8BF8qP/mIj+CwcpN92DeDs6oqPoJflDfst4a8wQVV9HwDHFNV8RF8rTxMFxL2zWCTKnq+QSwiFHAELp/nm8G5yoqPxPv05HlJNcI+DfxcOc83g0+Sp2bVE98gPjbs8mXm4IlvgG8rkfNzkVRkso6EyXbPYUuJmXTIak74LRZeQr+B4WR1mO8S+SwpcS38xqYE+4RzyXP2TrxPfoOJv8MsD4OEmWvpscSrsZByOXnxHaPkZFJeG+l1P2tqampqwmw+AO2Rw/WiIGDFAAAAAElFTkSuQmCC') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Sports & Outdoors', 'Sporting Equipment;Outdoors & Camping;Hiking;Hunting;Fishing;Biking', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABmJLR0QA/wD/AP+gvaeTAAAEPElEQVR4nO2a34sVZRjHv0+aGbEJ/boo291ItwQTFOumGyP6edWvTQsCY8OLCrzrOnRRwrv+AV2NfghhGwTSTd0VsSW1IpFFF5ViJbWK7rrqfrqYM3Cc887sO3Pmnfdo53N1ZuZ9n+/zPPP+mHnmSH369OnzP8ZiO+ACuF3SS5LWt059J+kDM/s7nlcNAWwGztDJDDAa27+gAE8AlxzBp1wCHovtZxAAA34sCD7lGNCTU7crgHUewaesrUv3uroM1cBwibb31CXaSwn4t0Tbf4J5EQvgxpzVP8sMsLwu3Z4ZAWY2K2mPR9N3zGwutD9RAJYA7xfc/feAJbH9DArJdvgy8CVwtjUtvgC2XDPbH7AM2AH80bqzl1v7+5bYvgUHWAMcKRjmb8f2MRjAg8BfHiv99ti+1g7wCH7bXDolnovtc20AzwMXPINPOQc8FNv3rgFeAOZLBp9yErgrdgyVAUaBixWDT/kKuCF2LKWpKfiUidjxlAJ4scbgU96MHZcXwDMUV3WqMg9sqtvfWh8tgfslTUm6qaKJI5KWS1qTc31G0m5J+yTNF9g5Z2ZF18MAfNLFHf4cGABWAr92OVqaL54Ct1B93k8A17fZuhc40UQC6qwHrJK0tEK/dyVtNbOL6Qkz+0XS45JO1+RbLnUm4GzJ9pclvW5m282M7EUzOyrpaUln6nAuOCTFjDJzd8zT7kZgKtQUqHsXeFbSx552j0p61Mz+9LQ9JGlI0jKP5tNmdsrHbu0Ar5C8yPgwDdwRxdGQAIPARyWScGtsn4MAbAK+90jCZ7F9DQawFHgDOL1IEh6O4V/pRRBYIelJSWslrWyd/l3StKTDZubctoDbJI1Lek2Sq7S9x8zeCqXfNcAwcIDiys4FYD/Jip1nZz1w3NH3wyb0qwa/DZhbZAi3M0vBPg8cdPQ52JR+2eB3lRDOMt5tAkLolwl+W47hBeAbkpeYidbvhZy2HXfCNwGh9H2DH8Y97A4BqxztVwOTjvazwGDZBITU903AAYexcQq+z5F813MN2X0VEhBM3yf4FXSutoeKxDNOZO/EHDDgm4DQ+j4J2JwxsIBj2BX0H6FzTo62XV8sAUH128mrBzyQOZ4ys599HTCzn5T8ubGddb79m9TPS8CdmeNjvuIFfbI2i2hMPy8B2QpNlbpBtk9H1aeAxvTzEnAyc5xXpi4i2+dEib6N6ecVMX/IHG8EVpvZcR9l4D5JGwpsfu3o1n4utP6iBm52bEOTJbahT7vZhmLrp4YmHFvVriInWuK7Hf32lhLvAX0BQySPkVkmgRFH+xFH5gHOA3dfbfqp0TGHQUgeMqZI3r33A9+S/zLyaiXxHtBPnRjPMezDzq7Ee0A/dWIM93DM4zywtRbxHtBPnRgkWZiKKjNzwF6qzrkI+lWKogOSntKVRcnflHzpOWxmZb8RXlX6ffr06XNN8R8f+FLdSRxelgAAAABJRU5ErkJggg==') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Real Estate, Property & Housing', '', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAACXBIWXMAAAsTAAALEwEAmpwYAAAC4ElEQVR4nO2ZPWgUURDHN1FEQQ0RIxiRoE0g2IiNgfRaGBRjSLAINn4UVkGwUJtUIY2VgYhgJ3jgV6EgEQIWNqIighFUEFQQsdDgF8b4k4ezOLfs3u3d7t7t5uYHw3HzZt6+98+8j9t4nmEYhmEYhgIosfwoeXHBBMAqgOWHLQHP9oB0NsF5YBToktiNwBjwpkLOe2CX2JGImN8qZrBKOe9TsWE2l9USeAasj8jpAl5H5E2ruJXAp5CYRRWzrYoAPVXGfyMrAQ6omL3ARfepfCMReXsC/V9OSwDgKHAsYE+yEqBb2lcD38X3DVgr/nUhOZ+BVYH+96cogFs6cUhFgC3SvgL4qPxXgRmx4ICuqH4H5HMN8LWIAhxSMTuB+zEePCLxG9zaVPnXiyjAS7frB2J3A7cj4n/6myZwEPjiNkH5PpaGACFjP5elAMhxNxiS4yb0i3LuqPYL4utXFbFYRAF8HgNDgbwpyjmu2l6I76zy3UthCdwFZpW9avRNcEblbVX+JWCzatsutkn5ThZpD5hV7e6s1ehJ+cvgQYznOMH+FEWAd0C7GuCC+D+oja1DxZ9W/Q2578r6VNvDogjgGFUx3e5m6N8NxHeY//Qq/1vKmVJtZ0IEaAc6A9YTIUBniN0kIwEWgtdalbNDqsHxXPl7Q/p5pNr7ggJE9N9Rw2+BUtab4BxwSu7+J4Brcub7PAUmxW6F5C/JieHH/BDfZAU7r/Knq8TOZy1AETEBPKuAmGBLgJbfA8alCrKy4L0A8WX5zHEvL5D0nC46mABYBWBLoMB7ANCmXnFVJGkFxHkGcMn/qZ6ryTsaJEBjRKDGyTdYgGxFoI7JN0GAbESgzsk3SYB0ReDf5N2/vOoios9+YDhg/SkKgIy5rWl/eZ9EA0gmQLJKIIXJ50CA+kQgpcnnRIDaRQAmUnpwXgRwTDTrhchwQksLeyfoNakC8oIJ4FkFxIRWXwKGYRiGYXitwF+FAy8M28uvbgAAAABJRU5ErkJggg==') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Luggage & Travel', '', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABmJLR0QA/wD/AP+gvaeTAAAC9klEQVR4nO2bPWjVUBiG38+K2C7iLzhZVyc7O/iDojhY3HSxautYFQQXF0HB2cEiKCLqIEoFCwoiaKU4V0REKFpxrFY61EFrfRyuQr3kJLnJPTkxNw90yTnN95435+R+J/ki1dTU1HQw5uOkwFFJ+5sOz0gaNjN8xCwVwAGiORFaWyEA3cB8hAFzwMbQ+goBGHXMgjuhtRUCcMRhAEDz/aF6AGuABYcBU0B3aI3eAZ7FzIKLofV5BzgVY8B3YEtojV4BNgG/YkyYALzkIqUBmIwxAGAotEavAOcTDJgFNoTW6Q1ga4IBALdD6/QK8CGFCXtCaFtWUJyxFH1GqGpuAOxMMQMALoTW6gWgC/icwoDCc4PIJQAMAveW/A3mCWJmi5Iep+i6QtJVQucGwLGmK7MAbMt5zoMplwF5Dc8NsDlC1CdgXY5z9gDfUhoQPjcAPkYIe0SO6Qk8bGEW3GrneLKIvekQdibHOY+3YADA7naOKYrlMW3jkgYijl8C5iV9zRBvZYv9R4BzGeL8kDRtZq+TOjqnM9AraTpD8DLxTtJJM3vq6hC7noFpSb3tVlUwi5IOm9n9qMakVHi8/XoKp0vSNRy/YEkGvGi/niCsknQoqqETZsBf+qIOFrUbLAORr+SSDNjhQUgoJqMOJhmw3YOQEMxJuhvV0AkzYFHSkJnNRjU6DfiTCP3vOcBbSXvNbNTVIS4Vdl39BUnDypYKr5d0pYX+U5KypsLvzexNhv9t4GkzNFi2zVCcWB/b4bEWBh9uO0wHPRBx3QSb1/9PNTYUX3LE2iepJ2Xfs2Y2kyNWalw3wWWSlu6enpjZy5yx+lP2m5B0I2esckGJH4sXArAr5dqv7IuRyykGX92yGUr8ctQ7QF+KwVf39Th1gQSvEgyobokMjSKpOKpdJAWcjhl8NX/zlwI8jzGg2oWSwFo6uVQWGIi5+h1RLP3AMfjql8vT6R9MAP2Oq1+6T2biHormYbX+fZ4gNT6auu4pXk1NTU1NFn4DHMdp490i7dkAAAAASUVORK5CYII=') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Business, Industrial & Scientific', '', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABmJLR0QA/wD/AP+gvaeTAAADDUlEQVR4nO2ZvWsUQRyG30lSaCJiVLSKYAoxooJIIIWiWImFYJcEYmnhPyD4B6iNpcFCUAjEkAQsRC20ERTxA4JFijSCImgR/IhJUHPJY3GJKN7uzsztZO5knubgbnbe9/fu7MzsnJRIJBKJRCLRwABbgBd4ENt73dRTvE0AbY5mdknqltQpaVbSlDFm3q80K71OSQ8lHQ6lYWOiFTgLvK4R8BJwBzgUQLcTeOV7521HQJGJrcAjC50KcKGk2ksr3iaAzEcA2CDpnqQ+C8+tkq4AFWPMVftS/9JrkXRK0vHVz56MppOSxn00XA1d8gj8J7DfQ6sLeG7R/xjgNG95QXXoL3gEAHDbUWsTMN0wxa+aGvIsPhTjBCq+JeP7eMvOv0xIGjTGVEJ0nhXA9hBiHjxVwOKl7AAWQgk60iepP6RAVgAzIUUdaJV0CxgIJWBqfQn0SJrO+r2AKUmXHdp3SLomqT2nzbKkIWOM0wpTF8B9zxn7tIfWGYqX3QowGKLWLFPdwFfH4icBn1EjoAe4CbwFlhslhKPAF8viPwB5w9hF9xgw7xh+Tcowsxu4C6wUaK0A+0qof023lBDK8iNgDzBcoHe9NEGVE0KZfgS0A7M5eovAtpI1jwBzoQLI2gfUxBizKOlGTpONks659Gmh+UTV1+NvZfb7u3/XC4AuSW+UfZbwXlK3MWapHmM1dA9I2ut6nTFmokwfa2bGCkZesJ1bQwD0FgTwMrbH4ADPCkKwOUprXoD+ggDWb98eA6ANeJcTwBLVCbOhcVoG/2T1kGI4p0mbpPO+/TcFVM/v83Zqn4CO2D7z8B4BkmSM+SxpJKdJp6ShejQaHqrvCFmvrwAzVP/0+H8BHhSsCCdje6wb4CAwCnwsKHa9mQceAwN4HsbYFD8I/Ihaph0jlP24Ub3zzVD8Ghdd6iscMsCopGZ6uZmTtNMY892msc1wOVGfn3Vns6Re28Y2Aezw9xINa882AYSZWcNiPRH+3xsUC1IAsQ3EJgUQ20BsUgCxDcQmBRDbQCKRSCQSiUQsfgGI7rZpsWTP5QAAAABJRU5ErkJggg==') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Illegal', 'Banned and/or prohibited items', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABmJLR0QA/wD/AP+gvaeTAAAFDUlEQVR4nO2bzW9WRRTGn2ljEaVFSghl40fStC5cYcNOSXVjQmo0fqAsWGI1oWyQmFQihkDUFQQ2xqVGoqa6smGPfwENkmoMpUHBapR+CG2I/bmYt0rrmfs5975NeJ/lvTPnOee5c+ecOzNXaqGFFlq4h+HqIgK2S3pCUr+kRyQ9JGlT4/aCpD8lXZX0g6QJ59xMXb5VAqANeAY4C3xPflwCzgCDQFuz48kMYCvwHjBdIOgQrjZsdjc7viCAzcBHwHzEwNdiHvgQ6Gp2vKsA7AOuVxj4WvwCvB7D91KTYONJfCLp1RzdFiRNSbohaV5Sm6QHJfVI6pV0fw5b5yS94Zybz9EnDoDHgR8zPK1FYBw4DOwkYULDT5z9wAFgrNE3DZNAX52xC9gF/Jbi2AzwPj79FeXZAozgJ8E0roGYMSY5tYvkie4WcATIM5TTODuAg8DNBN65ykXAD/ukJ3+BCocj0IN/nZJGQjX8QBfJ7/wyMFQJ+Wo/HDDa4LMwCXRWQfxFQvArWAT2RCe3/dkP3An48Xlssn0Zgm+WCKGRsDcWyWbCRU6IvE4RRgM+/EyMVwFf3lq4AAwRztW1iICvHUIT48myxrdip7xbNGZbYM86EKEHO0XOUeYDCjgWCOztNe3WgwgjAf6jRQ22YX/SzgAbjfbPAbcDTiwBz5eOMtnf+4Apg3uKIusJwLOBYI4l9GnqSAAOBbh3FzF21jB0m5TavpkiAN0B7tNFjFnLWOMZ+zZThG8Mzom8RrZj5/jDOWw0RQRg2OBbBrblMRJ6/3fmdKZ2EfAfbBYG8xh5yzAwT4HZtG4RgHbsbDRstQ8F9Khxbco5t5zXIefct5JekrRk3N4gaSymCM65vyX9ZNyyYgoKYK26Xi/hVK0iSLI2VTZbDUMCbDKu/VXYHf0rwguSFo3bGyR9TbxiyVoktWIKCmCh9Daac+68pBdli9Ah6atIIpC1YUiABeNalFWWhggvy34dOiR9GeF1sJ62FVNQgDnjWvY8moIa5oQdxrVZq2FIgCnjWm+RNBhCVSIA7QpksTxGQoVQf16HMnBFrROAgYCtXIVQqBQ+kDvCbHzRRMDvSaxFvlK4YeiSYWgsd3TZ+aKIAJw3+l8s4tCZgCNbchvLzllKBPzSmNX/VBFnBgOOjBSOMBtvYRGA44F+TxdxxAFXDGPTQEepKNO5cy+vAQ9gb9tNAcWKOPyxFAsHS0eZzp1rJADvBNoWWxRtGO3GLy2vxU2gp3SU6fyZRMCfKbBGzCxl5yzgg4AD49RweiuDCEPAd4H7J2I40IU/k2NhNEKMWXxIEiG0PXeNWLvEwGsJ5PujkKT7kCSChVdiO3AuQHRnHYrwaRXknfjDB6GRMErRdJPPjyHCwx7gMmAufsQg78NvjYUwToXZAT/bhyY8gF+B3qr4V5wYwE6NK7iJ36iMViwBG/F5PlQcgU95T8biTHNoIGUkgK8YD1Fiexpf2x8n/UjejdqCv8u5PsJzwt1YxG9XDeM3LdoTbLY3xD2C/6rLMuFdpsSwL3tUtlPSx5LynNtdkl+3Xzkqi/x64w5Jj0n639Z7Aj6T9KZzzlzvqw3AXvyZnLpwjdh5vizwafIkyRNkWcwCJ6jiHGAs4D+gjmKf1iiKK8C7VLgYEx349YTdwGlgguTiZS2WgYvAKeApKiyw6vxpapv++2nqYUndWv3T1B+SpiVNyv809XtdvrXQQgst3LP4B1dCXq0t0BR7AAAAAElFTkSuQmCC') RETURNING id;");
    category_id = database->get_integer("INSERT INTO categories (name, description, thumbnail) VALUES ('Miscellaneous', 'Others;Non-classified', 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAACXBIWXMAAAsTAAALEwEAmpwYAAACbUlEQVR4nO2aT4oTQRSHS5mlbkTRCyg4oxdQZ+sJTFzPKUTcCK7nAEJWji4mzBHcZDMqguNC8ABKZlZDsjIb88ljSgjdVd1J6k+69H3QkG5eNfX7db1X1Z0yRlEURVGUTQBcMqUA3APeAWd0l1Pbx53Y4p8CM8pB+tqLJf5+YeL/MosyErgYUqVyEMOAM8rlNIYB80yd/SB5G3DsO+45j2FALoaB/RQTaqgBoZAPHQEm7EFpCqA1oI7WgFDIh9YAE/agtAagNaCO1oBQyIfWABP2oLQGUHgN+CFpEHAcl25AEtSAUCgcNSAU3Bw3FKOvwAR43xAjBa+tCI5aHu5ok0Ww54l9tfAV+RzY9cRJh73rAOAW8L1B/Ovq/4q51wE9R9wdxyf0L6sasI74rhjw2BE3WcWAdcV3xYBrdtgvcriCAaN1xdt79jdqgCA5L8PeFsFD4LpZ3gACxHtHjkllAPACGNvjeUP7G8ARMAVOrEnDHOJTGnDkuNb3tK/GntspMrn4lAZMHdcGnvau2EkO8SkNOHFcc6aBI3buaR9dfEoD9oBPC+cfgSue9rsLs4OIf9lSA6KJT10Et4CHwAP5bWOv2uI4kH1FlSlS1gm37fkwh/ikBph6nBjyecm08K0DQsTnnQZNPe6RI268ggHeb4LATeBbg/jBxhdCXKRDlZ+hBiwp/nIXlsJblcIoPAsxYJV3g5QGzB333fdsVNqzU9zUivRtaJINUW2bpPpL5Hy/ZZPU7/99m9w4hgFvKZc3MQzYKXSr7C9gO9gAwebarDDxT0xMgG2bDvL621WkbwfA3ajiFUVRFEUx/yJ/AKs6hY9h/vEkAAAAAElFTkSuQmCC') RETURNING id;");
    //category_id = database->get_integer("INSERT INTO categories (name, description) VALUES ('Collectables & Art', '') RETURNING id;");
    //category_id = database->get_integer("INSERT INTO categories (name, description) VALUES ('', '') RETURNING id;");
    // NOTE: Categories also act as tags to be used for filtering specific products
    }
    //-------------------------
    database->execute("COMMIT;");
}
//----------------------------------------------------------------
std::string neroshop::Backend::getDatabaseHash() {
    // Get contents from data.sqlite3 file
    std::ifstream rfile (std::string("data.sqlite3").c_str(), std::ios::binary);
    std::stringstream db_content;
    db_content << rfile.rdbuf(); // dump file contents
    rfile.close();
    // Get SHA256sum of data.sqlite3 contents
    std::string sha256sum;
    Validator::generate_sha256_hash(db_content.str(), sha256sum);
    std::cout << "sha256sum (data.sqlite3): " << sha256sum << std::endl;
    return sha256sum; // database may have to be closed first in order to get the accurate hash
}
//----------------------------------------------------------------
//----------------------------------------------------------------
QVariantList neroshop::Backend::getCategoryList(bool sort_alphabetically) const {
    // Do some database reading to fetch each category row (database reads do not require consensus)
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), (sort_alphabetically) ? "SELECT * FROM categories ORDER BY name ASC;" : "SELECT * FROM categories ORDER BY id ASC;", -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    // Check whether the prepared statement returns no data (for example an UPDATE)
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::print("No data found. Be sure to use an appropriate SELECT statement", 1);
        return {};
    }
    
    QVariantList category_list;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        QVariantMap category_object; // Create an object for each row
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));////if(sqlite3_column_text(stmt, i) == nullptr) {throw std::runtime_error("column is NULL");}
            //std::cout << column_value << std::endl;//std::cout << sqlite3_column_name(stmt, i) << std::endl;
            if(i == 0) category_object.insert("id", QString::fromStdString(column_value).toInt());
            if(i == 1) category_object.insert("name", QString::fromStdString(column_value));
            if(i == 2) category_object.insert("description", QString::fromStdString(column_value));
            if(i == 3) category_object.insert("thumbnail", QString::fromStdString(column_value));      
            //if(i == ) category_object.insert("", QString::fromStdString(column_value)); 
        }
        category_list.append(category_object);
    }
    
    sqlite3_finalize(stmt);

    return category_list;
}

//----------------------------------------------------------------
int neroshop::Backend::getCategoryIdByName(const QString& category_name) const {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // Execute sqlite3 statement
    int category_id = database->get_integer_params("SELECT id FROM categories WHERE name = $1;", { category_name.toStdString() });
    return category_id;
}
//----------------------------------------------------------------
int neroshop::Backend::getCategoryProductCount(int category_id) const {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // Execute sqlite3 statement
    int category_product_count = database->get_integer_params("SELECT COUNT(*) FROM products WHERE category_id = $1;", { std::to_string(category_id) });
    return category_product_count;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
QVariantList neroshop::Backend::registerProduct(const QString& name, const QString& description,
        double weight, const QString& attributes, 
        const QString& product_code,
        int category_id) const 
{
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    QString product_uuid = QUuid::createUuid().toString();
    product_uuid = product_uuid.remove("{").remove("}"); // remove brackets
    
    std::string product_id = database->get_text_params("INSERT INTO products (uuid, name, description, weight, attributes, code, category_id) "
	    "VALUES ($1, $2, $3, $4, $5, $6, $7) "
	    "RETURNING uuid", { product_uuid.toStdString(), name.toStdString(), description.toStdString(), std::to_string(weight), attributes.toStdString(), product_code.toStdString(), std::to_string(category_id) });
    if(product_id.empty()) return { false, "" };
    
    if(product_code.isEmpty()) database->execute_params("UPDATE products SET code = NULL WHERE uuid = $1", { product_uuid.toStdString() });
    if(attributes.isEmpty()) database->execute_params("UPDATE products SET attributes = NULL WHERE uuid = $1", { product_uuid.toStdString() });
    return { true, QString::fromStdString(product_id) };
}
//----------------------------------------------------------------
void neroshop::Backend::uploadProductImage(const QString& product_id, const QString& filename) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    database->execute("BEGIN;");
    // Read image from file and retrieve its contents
    std::ifstream product_image_file(filename.toStdString(), std::ios::binary); // std::ios::binary is the same as std::ifstream::binary
    if(!product_image_file.good()) {
        std::cout << NEROSHOP_TAG "failed to load " << filename.toStdString() << std::endl; 
        database->execute("ROLLBACK;"); return;
    }
    product_image_file.seekg(0, std::ios::end);
    size_t size = static_cast<int>(product_image_file.tellg()); // in bytes
    // Limit product image size to 12582912 bytes (12 megabyte)
    // Todo: Database cannot scale to billions of users if I am storing blobs so I'll have to switch to text later
    if(size >= 12582912) {
        neroshop::print("Product upload image cannot exceed 12 MB (twelve megabyte)", 1);
        database->execute("ROLLBACK;"); return;
    }
    product_image_file.seekg(0);
    std::vector<unsigned char> buffer(size);
    if(!product_image_file.read(reinterpret_cast<char *>(&buffer[0]), size)) {
        std::cout << NEROSHOP_TAG "error: only " << product_image_file.gcount() << " could be read";
        database->execute("ROLLBACK;"); // abort transaction
        return; // exit function
    }
    product_image_file.close();
    // Store image in database as BLOB
    std::string command = "INSERT INTO images (product_id, name, data) VALUES ($1, $2, $3);";
    sqlite3_stmt * statement = nullptr;
    int result = sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        database->execute("ROLLBACK;"); return;
    }
    
    std::string product_uuid = product_id.toStdString();
    result = sqlite3_bind_text(statement, 1, product_uuid.c_str(), product_uuid.length(), SQLITE_STATIC);
    if(result != SQLITE_OK) {
        neroshop::print("sqlite3_bind_text (arg: 1): " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(statement);
        database->execute("ROLLBACK;"); return;
    }    
    std::string product_image_filename = filename.toStdString();
    result = sqlite3_bind_text(statement, 2, product_image_filename.c_str(), product_image_filename.length(), SQLITE_STATIC);
    if(result != SQLITE_OK) {
        neroshop::print("sqlite3_bind_text (arg: 2): " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(statement);
        database->execute("ROLLBACK;"); return;
    }        
    result = sqlite3_bind_blob(statement, 3, buffer.data(), size, SQLITE_STATIC);
    if(result != SQLITE_OK) {
        neroshop::print("sqlite3_bind_blob (arg: 3): " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(statement);
        database->execute("ROLLBACK;"); return;// nullptr;
    }    
    
    result = sqlite3_step(statement);
    if (result != SQLITE_DONE) {
        neroshop::print("sqlite3_step: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
    }
    sqlite3_finalize(statement);
    
    database->execute("COMMIT;");
}

//----------------------------------------------------------------
QVariantList neroshop::Backend::getProductImages(const QString& product_id) {
    // Do some database reading to fetch each category row (database reads do not require consensus)
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    std::string command = "SELECT id, name FROM images WHERE product_id = $1";////std::string command = "SELECT id, name, data FROM images WHERE product_id = $1";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    // Bind product_id to TEXT
    std::string product_uuid = product_id.toStdString();
    if(sqlite3_bind_text(stmt, 1, product_uuid.c_str(), product_uuid.length(), SQLITE_STATIC) != SQLITE_OK) {
        neroshop::print("sqlite3_bind_text (arg: 1): " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(stmt);
        return {};//database->execute("ROLLBACK;"); return {};
    }        
    // Check whether the prepared statement returns no data (for example an UPDATE)
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::print("No data found. Be sure to use an appropriate SELECT statement", 1);
        return {};
    }
    
    QVariantList product_image_list;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        QVariantMap product_image_object; // Create an object for each row
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));////if(sqlite3_column_text(stmt, i) == nullptr) {throw std::runtime_error("column is NULL");}
            if(i == 0) product_image_object.insert("image_id", QString::fromStdString(column_value).toInt());
            if(i == 1) product_image_object.insert("name", QString::fromStdString(column_value));
            ////if(i == 2) product_image_object.insert("data", QString::fromStdString(column_value));
        }
        product_image_list.append(product_image_object);
    }
    return product_image_list;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
int neroshop::Backend::getProductStarCount(const QString& product_id) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // Get total number of star ratings for a specific product
    unsigned int ratings_count = database->get_integer_params("SELECT COUNT(*) FROM product_ratings WHERE product_id = $1", { product_id.toStdString() });
    return ratings_count;
}
//----------------------------------------------------------------
int neroshop::Backend::getProductStarCount(const QString& product_id, int star_number) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // Get total number of N star ratings for a specific product
    if(star_number > 5) star_number = 5;
    if(star_number < 1) star_number = 1;
    unsigned int star_count = database->get_integer_params("SELECT COUNT(stars) FROM product_ratings WHERE product_id = $1 AND stars = $2", { product_id.toStdString(), std::to_string(star_number) });
    return star_count;
}
//----------------------------------------------------------------
float neroshop::Backend::getProductAverageStars(const QString& product_id) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // Get number of star ratings for a specific product
    unsigned int total_star_ratings = database->get_integer_params("SELECT COUNT(*) FROM product_ratings WHERE product_id = $1", { product_id.toStdString() });
    if(total_star_ratings == 0) { /*neroshop::print("This item has no star ratings", 2);*/ return 0.0f; }
    // Get number of 1, 2, 3, 4, and 5 star_ratings
    int one_star_count = database->get_integer_params("SELECT COUNT(stars) FROM product_ratings WHERE product_id = $1 AND stars = $2", { product_id.toStdString(), std::to_string(1) });
    int two_star_count = database->get_integer_params("SELECT COUNT(stars) FROM product_ratings WHERE product_id = $1 AND stars = $2", { product_id.toStdString(), std::to_string(2) });
    int three_star_count = database->get_integer_params("SELECT COUNT(stars) FROM product_ratings WHERE product_id = $1 AND stars = $2", { product_id.toStdString(), std::to_string(3) });
    int four_star_count = database->get_integer_params("SELECT COUNT(stars) FROM product_ratings WHERE product_id = $1 AND stars = $2", { product_id.toStdString(), std::to_string(4) });
    int five_star_count = database->get_integer_params("SELECT COUNT(stars) FROM product_ratings WHERE product_id = $1 AND stars = $2", { product_id.toStdString(), std::to_string(5) });
    // Now calculate the average stars 
    float average_stars = (
        (1 * static_cast<float>(one_star_count)) + 
        (2 * static_cast<float>(two_star_count)) + 
        (3 * static_cast<float>(three_star_count)) + 
        (4 * static_cast<float>(four_star_count)) + 
        (5 * static_cast<float>(five_star_count))) / total_star_ratings;
    return average_stars;
}
    
//----------------------------------------------------------------
//----------------------------------------------------------------
QString neroshop::Backend::getDisplayNameById(const QString& user_id) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    std::string display_name = database->get_text_params("SELECT name FROM users WHERE monero_address = $1", { user_id.toStdString() });
    return QString::fromStdString(display_name);
}
//----------------------------------------------------------------
//----------------------------------------------------------------
int neroshop::Backend::getCartMaximumItems() {
    return neroshop::Cart::get_max_items();
}
//----------------------------------------------------------------
int neroshop::Backend::getCartMaximumQuantity() {
    return neroshop::Cart::get_max_quantity();
}
//----------------------------------------------------------------
int neroshop::Backend::getStockAvailable(const QString& product_id) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    int quantity = database->get_integer_params("SELECT quantity FROM listings WHERE product_id = $1 AND quantity > 0", { product_id.toStdString() });
    return quantity;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
QVariantList neroshop::Backend::getInventory(const QString& user_id) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    std::string command = "SELECT DISTINCT * FROM listings JOIN products ON products.uuid = listings.product_id JOIN images ON images.product_id = listings.product_id WHERE seller_id = $1 GROUP BY images.product_id;";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    // Bind seller_id to TEXT
    std::string seller_uuid = user_id.toStdString();
    if(sqlite3_bind_text(stmt, 1, seller_uuid.c_str(), seller_uuid.length(), SQLITE_STATIC) != SQLITE_OK) {
        neroshop::print("sqlite3_bind_text (arg: 1): " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(stmt);
        return {};
    }    
    // Check whether the prepared statement returns no data (for example an UPDATE)
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::print("No data found. Be sure to use an appropriate SELECT statement", 1);
        return {};
    }
    
    QVariantList inventory_array;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        QVariantMap inventory_object; // Create an object for each row
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));////if(sqlite3_column_text(stmt, i) == nullptr) {throw std::runtime_error("column is NULL");}
            ////std::cout << column_value  << " (" << i << ")" << std::endl;//std::cout << sqlite3_column_name(stmt, i) << std::endl;
            // listings table
            if(i == 0) inventory_object.insert("listing_uuid", QString::fromStdString(column_value));
            if(i == 1) inventory_object.insert("product_id", QString::fromStdString(column_value));
            if(i == 2) inventory_object.insert("seller_id", QString::fromStdString(column_value));
            if(i == 3) inventory_object.insert("quantity", QString::fromStdString(column_value).toInt());
            if(i == 4) inventory_object.insert("price", QString::fromStdString(column_value).toDouble());
            if(i == 5) inventory_object.insert("currency", QString::fromStdString(column_value));
            if(i == 6) inventory_object.insert("condition", QString::fromStdString(column_value));
            if(i == 7) inventory_object.insert("location", QString::fromStdString(column_value));
            if(i == 8) inventory_object.insert("date", QString::fromStdString(column_value));
            // products table
            if(i == 9) inventory_object.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 10) inventory_object.insert("product_name", QString::fromStdString(column_value)); 
            if(i == 11) inventory_object.insert("product_description", QString::fromStdString(column_value)); 
            if(i == 12) inventory_object.insert("weight", QString::fromStdString(column_value).toDouble()); 
            if(i == 13) inventory_object.insert("product_attributes", QString::fromStdString(column_value)); 
            if(i == 14) inventory_object.insert("product_code", QString::fromStdString(column_value)); 
            if(i == 15) inventory_object.insert("product_category_id", QString::fromStdString(column_value).toInt()); 
            // images table
            //if(i == 16) inventory_object.insert("image_id", QString::fromStdString(column_value)); 
            //if(i == 17) inventory_object.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 18) inventory_object.insert("product_image_file", QString::fromStdString(column_value)); 
            //if(i == 19) inventory_object.insert("product_image_data", QString::fromStdString(column_value));        
        }
        inventory_array.append(inventory_object);
    }
    
    sqlite3_finalize(stmt);

    return inventory_array;
}
int neroshop::Backend::getListingsTotalResults(neroshop::QListingSearchParamsWrapper* wrapper) {
    if(!wrapper) {
        neroshop::print("Invalid QListingSearchParamsWrapper passed to getListingsTotalResults.", 1);
        return 0;
    }
    return neroshop::Searcher::listing_search_total_results(wrapper->getParams());
}
QVariantList neroshop::Backend::getListings(neroshop::QListingSearchParamsWrapper* wrapper) {
    QVariantList catalog_array;
    if(!wrapper) {
        neroshop::print("Invalid QListingSearchParamsWrapper passed to getListings.", 1);
        return catalog_array;
    }
    std::vector<Listing> listings = neroshop::Searcher::listing_search(wrapper->getParams());
    for(Listing listing : listings){
        catalog_array.append(listing.as_q_map());
    }
    return catalog_array;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
QVariantList neroshop::Backend::getListings() {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    std::string command = "SELECT DISTINCT * FROM listings JOIN products ON products.uuid = listings.product_id JOIN images ON images.product_id = listings.product_id GROUP BY images.product_id;";//WHERE stock_qty > 0;"; // image.product_id must be unique in this field to prevent duplicate listings!
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    // Check whether the prepared statement returns no data (for example an UPDATE)
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::print("No data found. Be sure to use an appropriate SELECT statement", 1);
        return {};
    }
    
    QVariantList catalog_array;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        QVariantMap listing; // Create an object for each row
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));////if(sqlite3_column_text(stmt, i) == nullptr) {throw std::runtime_error("column is NULL");}
            ////std::cout << column_value  << " (" << i << ")" << std::endl;//std::cout << sqlite3_column_name(stmt, i) << std::endl;
            // listings table
            if(i == 0) listing.insert("listing_uuid", QString::fromStdString(column_value));
            if(i == 1) listing.insert("product_id", QString::fromStdString(column_value));
            if(i == 2) listing.insert("seller_id", QString::fromStdString(column_value));
            if(i == 3) listing.insert("quantity", QString::fromStdString(column_value).toInt());
            if(i == 4) listing.insert("price", QString::fromStdString(column_value).toDouble());
            if(i == 5) listing.insert("currency", QString::fromStdString(column_value));
            if(i == 6) listing.insert("condition", QString::fromStdString(column_value));
            if(i == 7) listing.insert("location", QString::fromStdString(column_value));
            if(i == 8) listing.insert("date", QString::fromStdString(column_value));
            // products table
            if(i == 9) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 10) listing.insert("product_name", QString::fromStdString(column_value)); 
            if(i == 11) listing.insert("product_description", QString::fromStdString(column_value)); 
            if(i == 12) listing.insert("weight", QString::fromStdString(column_value).toDouble()); 
            if(i == 13) listing.insert("product_attributes", QString::fromStdString(column_value)); 
            if(i == 14) listing.insert("product_code", QString::fromStdString(column_value)); 
            if(i == 15) listing.insert("product_category_id", QString::fromStdString(column_value).toInt()); 
            // images table
            //if(i == 16) listing.insert("image_id", QString::fromStdString(column_value)); 
            //if(i == 17) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 18) listing.insert("product_image_file", QString::fromStdString(column_value)); 
            //if(i == 19) listing.insert("product_image_data", QString::fromStdString(column_value));
        }
        catalog_array.append(listing);
    }
    
    sqlite3_finalize(stmt);

    return catalog_array;
}
int neroshop::Backend::getCategoryListingCount(int category_id) {
    return neroshop::Searcher::listing_search_by_category_id_total_results(category_id);
}
//----------------------------------------------------------------
QVariantList neroshop::Backend::getListingsByCategory(int category_id) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    //std::string command = "SELECT DISTINCT * FROM listings ORDER BY date ASC;";// LIMIT $1;";//WHERE stock_qty > 0;";
    std::string command = "SELECT DISTINCT * FROM listings JOIN products ON products.uuid = listings.product_id JOIN images ON images.product_id = listings.product_id WHERE category_id = $1 GROUP BY images.product_id;";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    // Bind value to parameter arguments
    if(sqlite3_bind_int(stmt, 1, category_id) != SQLITE_OK) {
        neroshop::print("sqlite3_bind_int: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(stmt);
        return {};//database->execute("ROLLBACK;"); return {};
    }    
    // Check whether the prepared statement returns no data (for example an UPDATE)
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::print("No data found. Be sure to use an appropriate SELECT statement", 1);
        return {};
    }
    
    QVariantList catalog_array;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        QVariantMap listing; // Create an object for each row
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));////if(sqlite3_column_text(stmt, i) == nullptr) {throw std::runtime_error("column is NULL");}
            ////std::cout << column_value  << " (" << i << ")" << std::endl;//std::cout << sqlite3_column_name(stmt, i) << std::endl;
            // listings table
            if(i == 0) listing.insert("listing_uuid", QString::fromStdString(column_value));
            if(i == 1) listing.insert("product_id", QString::fromStdString(column_value));
            if(i == 2) listing.insert("seller_id", QString::fromStdString(column_value));
            if(i == 3) listing.insert("quantity", QString::fromStdString(column_value).toInt());
            if(i == 4) listing.insert("price", QString::fromStdString(column_value).toDouble());
            if(i == 5) listing.insert("currency", QString::fromStdString(column_value));
            if(i == 6) listing.insert("condition", QString::fromStdString(column_value));
            if(i == 7) listing.insert("location", QString::fromStdString(column_value));
            if(i == 8) listing.insert("date", QString::fromStdString(column_value));
            // products table
            if(i == 9) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 10) listing.insert("product_name", QString::fromStdString(column_value)); 
            if(i == 11) listing.insert("product_description", QString::fromStdString(column_value)); 
            if(i == 12) listing.insert("weight", QString::fromStdString(column_value).toDouble()); 
            if(i == 13) listing.insert("product_attributes", QString::fromStdString(column_value)); 
            if(i == 14) listing.insert("product_code", QString::fromStdString(column_value)); 
            if(i == 15) listing.insert("product_category_id", QString::fromStdString(column_value).toInt()); 
            // images table
            //if(i == 16) listing.insert("image_id", QString::fromStdString(column_value)); 
            //if(i == 17) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 18) listing.insert("product_image_file", QString::fromStdString(column_value)); 
            //if(i == 19) listing.insert("product_image_data", QString::fromStdString(column_value));
        }
        catalog_array.append(listing);
    }
    
    sqlite3_finalize(stmt);

    return catalog_array;
}
//----------------------------------------------------------------
QVariantList neroshop::Backend::getListingsByMostRecent() {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    //std::string command = "SELECT DISTINCT * FROM listings ORDER BY date ASC;";// LIMIT $1;";//WHERE stock_qty > 0;";
    std::string command = "SELECT DISTINCT * FROM listings JOIN products ON products.uuid = listings.product_id JOIN images ON images.product_id = listings.product_id GROUP BY images.product_id ORDER BY date DESC;";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    // Check whether the prepared statement returns no data (for example an UPDATE)
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::print("No data found. Be sure to use an appropriate SELECT statement", 1);
        return {};
    }
    
    QVariantList catalog_array;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        QVariantMap listing; // Create an object for each row
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));////if(sqlite3_column_text(stmt, i) == nullptr) {throw std::runtime_error("column is NULL");}
            ////std::cout << column_value  << " (" << i << ")" << std::endl;//std::cout << sqlite3_column_name(stmt, i) << std::endl;
            // listings table
            if(i == 0) listing.insert("listing_uuid", QString::fromStdString(column_value));
            if(i == 1) listing.insert("product_id", QString::fromStdString(column_value));
            if(i == 2) listing.insert("seller_id", QString::fromStdString(column_value));
            if(i == 3) listing.insert("quantity", QString::fromStdString(column_value).toInt());
            if(i == 4) listing.insert("price", QString::fromStdString(column_value).toDouble());
            if(i == 5) listing.insert("currency", QString::fromStdString(column_value));
            if(i == 6) listing.insert("condition", QString::fromStdString(column_value));
            if(i == 7) listing.insert("location", QString::fromStdString(column_value));
            if(i == 8) listing.insert("date", QString::fromStdString(column_value));
            // products table
            if(i == 9) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 10) listing.insert("product_name", QString::fromStdString(column_value)); 
            if(i == 11) listing.insert("product_description", QString::fromStdString(column_value)); 
            if(i == 12) listing.insert("weight", QString::fromStdString(column_value).toDouble()); 
            if(i == 13) listing.insert("product_attributes", QString::fromStdString(column_value)); 
            if(i == 14) listing.insert("product_code", QString::fromStdString(column_value)); 
            if(i == 15) listing.insert("product_category_id", QString::fromStdString(column_value).toInt()); 
            // images table
            //if(i == 16) listing.insert("image_id", QString::fromStdString(column_value)); 
            //if(i == 17) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 18) listing.insert("product_image_file", QString::fromStdString(column_value)); 
            //if(i == 19) listing.insert("product_image_data", QString::fromStdString(column_value));
        }
        catalog_array.append(listing);
    }
    
    sqlite3_finalize(stmt);

    return catalog_array;
}
//----------------------------------------------------------------
QVariantList neroshop::Backend::getListingsByMostRecentLimit(int limit) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    //std::string command = "SELECT DISTINCT * FROM listings ORDER BY date ASC;";// LIMIT $1;";//WHERE stock_qty > 0;";
    std::string command = "SELECT DISTINCT * FROM listings JOIN products ON products.uuid = listings.product_id JOIN images ON images.product_id = listings.product_id GROUP BY images.product_id ORDER BY date DESC LIMIT $1;";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    // Limit the number of item results (rows)
    if(sqlite3_bind_int(stmt, 1, limit) != SQLITE_OK) {
        neroshop::print("sqlite3_bind_int: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(stmt);
        return {};//database->execute("ROLLBACK;"); return {};
    }
    // Check whether the prepared statement returns no data (for example an UPDATE)
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::print("No data found. Be sure to use an appropriate SELECT statement", 1);
        return {};
    }
    
    QVariantList catalog_array;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        QVariantMap listing; // Create an object for each row
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));////if(sqlite3_column_text(stmt, i) == nullptr) {throw std::runtime_error("column is NULL");}
            ////std::cout << column_value  << " (" << i << ")" << std::endl;//std::cout << sqlite3_column_name(stmt, i) << std::endl;
            // listings table
            if(i == 0) listing.insert("listing_uuid", QString::fromStdString(column_value));
            if(i == 1) listing.insert("product_id", QString::fromStdString(column_value));
            if(i == 2) listing.insert("seller_id", QString::fromStdString(column_value));
            if(i == 3) listing.insert("quantity", QString::fromStdString(column_value).toInt());
            if(i == 4) listing.insert("price", QString::fromStdString(column_value).toDouble());
            if(i == 5) listing.insert("currency", QString::fromStdString(column_value));
            if(i == 6) listing.insert("condition", QString::fromStdString(column_value));
            if(i == 7) listing.insert("location", QString::fromStdString(column_value));
            if(i == 8) listing.insert("date", QString::fromStdString(column_value));
            // products table
            if(i == 9) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 10) listing.insert("product_name", QString::fromStdString(column_value)); 
            if(i == 11) listing.insert("product_description", QString::fromStdString(column_value)); 
            if(i == 12) listing.insert("weight", QString::fromStdString(column_value).toDouble()); 
            if(i == 13) listing.insert("product_attributes", QString::fromStdString(column_value)); 
            if(i == 14) listing.insert("product_code", QString::fromStdString(column_value)); 
            if(i == 15) listing.insert("product_category_id", QString::fromStdString(column_value).toInt()); 
            // images table
            //if(i == 16) listing.insert("image_id", QString::fromStdString(column_value)); 
            //if(i == 17) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 18) listing.insert("product_image_file", QString::fromStdString(column_value)); 
            //if(i == 19) listing.insert("product_image_data", QString::fromStdString(column_value));
        }
        catalog_array.append(listing);
    }
    
    sqlite3_finalize(stmt);

    return catalog_array;
}
//----------------------------------------------------------------
QVariantList neroshop::Backend::getListingsByOldest() {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    //std::string command = "SELECT DISTINCT * FROM listings ORDER BY date ASC;";// LIMIT $1;";//WHERE stock_qty > 0;";
    std::string command = "SELECT DISTINCT * FROM listings JOIN products ON products.uuid = listings.product_id JOIN images ON images.product_id = listings.product_id GROUP BY images.product_id ORDER BY date ASC;";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    // Check whether the prepared statement returns no data (for example an UPDATE)
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::print("No data found. Be sure to use an appropriate SELECT statement", 1);
        return {};
    }
    
    QVariantList catalog_array;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        QVariantMap listing; // Create an object for each row
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));////if(sqlite3_column_text(stmt, i) == nullptr) {throw std::runtime_error("column is NULL");}
            ////std::cout << column_value  << " (" << i << ")" << std::endl;//std::cout << sqlite3_column_name(stmt, i) << std::endl;
            // listings table
            if(i == 0) listing.insert("listing_uuid", QString::fromStdString(column_value));
            if(i == 1) listing.insert("product_id", QString::fromStdString(column_value));
            if(i == 2) listing.insert("seller_id", QString::fromStdString(column_value));
            if(i == 3) listing.insert("quantity", QString::fromStdString(column_value).toInt());
            if(i == 4) listing.insert("price", QString::fromStdString(column_value).toDouble());
            if(i == 5) listing.insert("currency", QString::fromStdString(column_value));
            if(i == 6) listing.insert("condition", QString::fromStdString(column_value));
            if(i == 7) listing.insert("location", QString::fromStdString(column_value));
            if(i == 8) listing.insert("date", QString::fromStdString(column_value));
            // products table
            if(i == 9) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 10) listing.insert("product_name", QString::fromStdString(column_value)); 
            if(i == 11) listing.insert("product_description", QString::fromStdString(column_value)); 
            if(i == 12) listing.insert("weight", QString::fromStdString(column_value).toDouble()); 
            if(i == 13) listing.insert("product_attributes", QString::fromStdString(column_value)); 
            if(i == 14) listing.insert("product_code", QString::fromStdString(column_value)); 
            if(i == 15) listing.insert("product_category_id", QString::fromStdString(column_value).toInt()); 
            // images table
            //if(i == 16) listing.insert("image_id", QString::fromStdString(column_value)); 
            //if(i == 17) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 18) listing.insert("product_image_file", QString::fromStdString(column_value)); 
            //if(i == 19) listing.insert("product_image_data", QString::fromStdString(column_value));
        }
        catalog_array.append(listing);
    }
    
    sqlite3_finalize(stmt);

    return catalog_array;
}
//----------------------------------------------------------------
QVariantList neroshop::Backend::getListingsByAlphabeticalOrder() {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    //std::string command = "SELECT DISTINCT * FROM listings ORDER BY date ASC;";// LIMIT $1;";//WHERE stock_qty > 0;";
    std::string command = "SELECT DISTINCT * FROM listings JOIN products ON products.uuid = listings.product_id JOIN images ON images.product_id = listings.product_id GROUP BY images.product_id ORDER BY products.name COLLATE NOCASE ASC;";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    // Check whether the prepared statement returns no data (for example an UPDATE)
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::print("No data found. Be sure to use an appropriate SELECT statement", 1);
        return {};
    }
    
    QVariantList catalog_array;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        QVariantMap listing; // Create an object for each row
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));////if(sqlite3_column_text(stmt, i) == nullptr) {throw std::runtime_error("column is NULL");}
            ////std::cout << column_value  << " (" << i << ")" << std::endl;//std::cout << sqlite3_column_name(stmt, i) << std::endl;
            // listings table
            if(i == 0) listing.insert("listing_uuid", QString::fromStdString(column_value));
            if(i == 1) listing.insert("product_id", QString::fromStdString(column_value));
            if(i == 2) listing.insert("seller_id", QString::fromStdString(column_value));
            if(i == 3) listing.insert("quantity", QString::fromStdString(column_value).toInt());
            if(i == 4) listing.insert("price", QString::fromStdString(column_value).toDouble());
            if(i == 5) listing.insert("currency", QString::fromStdString(column_value));
            if(i == 6) listing.insert("condition", QString::fromStdString(column_value));
            if(i == 7) listing.insert("location", QString::fromStdString(column_value));
            if(i == 8) listing.insert("date", QString::fromStdString(column_value));
            // products table
            if(i == 9) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 10) listing.insert("product_name", QString::fromStdString(column_value)); 
            if(i == 11) listing.insert("product_description", QString::fromStdString(column_value)); 
            if(i == 12) listing.insert("weight", QString::fromStdString(column_value).toDouble()); 
            if(i == 13) listing.insert("product_attributes", QString::fromStdString(column_value)); 
            if(i == 14) listing.insert("product_code", QString::fromStdString(column_value)); 
            if(i == 15) listing.insert("product_category_id", QString::fromStdString(column_value).toInt()); 
            // images table
            //if(i == 16) listing.insert("image_id", QString::fromStdString(column_value)); 
            //if(i == 17) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 18) listing.insert("product_image_file", QString::fromStdString(column_value)); 
            //if(i == 19) listing.insert("product_image_data", QString::fromStdString(column_value));
        }
        catalog_array.append(listing);
    }
    
    sqlite3_finalize(stmt);

    return catalog_array;
}
//----------------------------------------------------------------
QVariantList neroshop::Backend::getListingsByPriceLowest() {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    //std::string command = "SELECT DISTINCT * FROM listings ORDER BY date ASC;";// LIMIT $1;";//WHERE stock_qty > 0;";
    std::string command = "SELECT DISTINCT * FROM listings JOIN products ON products.uuid = listings.product_id JOIN images ON images.product_id = listings.product_id GROUP BY images.product_id ORDER BY price ASC;";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    // Check whether the prepared statement returns no data (for example an UPDATE)
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::print("No data found. Be sure to use an appropriate SELECT statement", 1);
        return {};
    }
    
    QVariantList catalog_array;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        QVariantMap listing; // Create an object for each row
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));////if(sqlite3_column_text(stmt, i) == nullptr) {throw std::runtime_error("column is NULL");}
            ////std::cout << column_value  << " (" << i << ")" << std::endl;//std::cout << sqlite3_column_name(stmt, i) << std::endl;
            // listings table
            if(i == 0) listing.insert("listing_uuid", QString::fromStdString(column_value));
            if(i == 1) listing.insert("product_id", QString::fromStdString(column_value));
            if(i == 2) listing.insert("seller_id", QString::fromStdString(column_value));
            if(i == 3) listing.insert("quantity", QString::fromStdString(column_value).toInt());
            if(i == 4) listing.insert("price", QString::fromStdString(column_value).toDouble());
            if(i == 5) listing.insert("currency", QString::fromStdString(column_value));
            if(i == 6) listing.insert("condition", QString::fromStdString(column_value));
            if(i == 7) listing.insert("location", QString::fromStdString(column_value));
            if(i == 8) listing.insert("date", QString::fromStdString(column_value));
            // products table
            if(i == 9) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 10) listing.insert("product_name", QString::fromStdString(column_value)); 
            if(i == 11) listing.insert("product_description", QString::fromStdString(column_value)); 
            if(i == 12) listing.insert("weight", QString::fromStdString(column_value).toDouble()); 
            if(i == 13) listing.insert("product_attributes", QString::fromStdString(column_value)); 
            if(i == 14) listing.insert("product_code", QString::fromStdString(column_value)); 
            if(i == 15) listing.insert("product_category_id", QString::fromStdString(column_value).toInt()); 
            // images table
            //if(i == 16) listing.insert("image_id", QString::fromStdString(column_value)); 
            //if(i == 17) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 18) listing.insert("product_image_file", QString::fromStdString(column_value)); 
            //if(i == 19) listing.insert("product_image_data", QString::fromStdString(column_value));
        }
        catalog_array.append(listing);
    }
    
    sqlite3_finalize(stmt);

    return catalog_array;
}
//----------------------------------------------------------------
QVariantList neroshop::Backend::getListingsByPriceHighest() {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    //std::string command = "SELECT DISTINCT * FROM listings ORDER BY date ASC;";// LIMIT $1;";//WHERE stock_qty > 0;";
    std::string command = "SELECT DISTINCT * FROM listings JOIN products ON products.uuid = listings.product_id JOIN images ON images.product_id = listings.product_id GROUP BY images.product_id ORDER BY price DESC;";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    // Check whether the prepared statement returns no data (for example an UPDATE)
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::print("No data found. Be sure to use an appropriate SELECT statement", 1);
        return {};
    }
    
    QVariantList catalog_array;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        QVariantMap listing; // Create an object for each row
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));////if(sqlite3_column_text(stmt, i) == nullptr) {throw std::runtime_error("column is NULL");}
            ////std::cout << column_value  << " (" << i << ")" << std::endl;//std::cout << sqlite3_column_name(stmt, i) << std::endl;
            // listings table
            if(i == 0) listing.insert("listing_uuid", QString::fromStdString(column_value));
            if(i == 1) listing.insert("product_id", QString::fromStdString(column_value));
            if(i == 2) listing.insert("seller_id", QString::fromStdString(column_value));
            if(i == 3) listing.insert("quantity", QString::fromStdString(column_value).toInt());
            if(i == 4) listing.insert("price", QString::fromStdString(column_value).toDouble());
            if(i == 5) listing.insert("currency", QString::fromStdString(column_value));
            if(i == 6) listing.insert("condition", QString::fromStdString(column_value));
            if(i == 7) listing.insert("location", QString::fromStdString(column_value));
            if(i == 8) listing.insert("date", QString::fromStdString(column_value));
            // products table
            if(i == 9) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 10) listing.insert("product_name", QString::fromStdString(column_value)); 
            if(i == 11) listing.insert("product_description", QString::fromStdString(column_value)); 
            if(i == 12) listing.insert("weight", QString::fromStdString(column_value).toDouble()); 
            if(i == 13) listing.insert("product_attributes", QString::fromStdString(column_value)); 
            if(i == 14) listing.insert("product_code", QString::fromStdString(column_value)); 
            if(i == 15) listing.insert("product_category_id", QString::fromStdString(column_value).toInt()); 
            // images table
            //if(i == 16) listing.insert("image_id", QString::fromStdString(column_value)); 
            //if(i == 17) listing.insert("product_uuid", QString::fromStdString(column_value)); 
            if(i == 18) listing.insert("product_image_file", QString::fromStdString(column_value)); 
            //if(i == 19) listing.insert("product_image_data", QString::fromStdString(column_value));
        }
        catalog_array.append(listing);
    }
    
    sqlite3_finalize(stmt);

    return catalog_array;
}
//----------------------------------------------------------------
QVariantList neroshop::Backend::getNodeListDefault(const QString& coin) const {
    QVariantList node_list;
    std::string network_type = neroshop::Script::get_string(neroshop::lua_state, "neroshop.monero.daemon.network_type");
    std::vector<std::string> node_table = neroshop::Script::get_table_string(neroshop::lua_state, "neroshop." + coin.toStdString() + ".nodes." + network_type); // Get monero nodes from settings.lua////std::cout << "lua_query: " << "neroshop." + coin.toStdString() + ".nodes." + network_type << std::endl;
    for(auto strings : node_table) {
        node_list << QString::fromStdString(strings);
    }
    return node_list;
}
//----------------------------------------------------------------
QVariantList neroshop::Backend::getNodeList(const QString& coin) const {
    const QUrl url(QStringLiteral("https://monero.fail/health.json"));
    QVariantList node_list;
    QString coin_lower = coin.toLower(); // make coin name lowercase
    
    QNetworkAccessManager manager;
    QEventLoop loop;
    QObject::connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    auto reply = manager.get(QNetworkRequest(url));
    loop.exec();
    QJsonParseError error;
    const auto json_doc = QJsonDocument::fromJson(reply->readAll(), &error);
    // Use fallback monero node list if we fail to get the nodes from the url
    if (error.error != QJsonParseError::NoError) {
        neroshop::print("Error reading json from " + url.toString().toStdString() + "\nUsing default nodes as fallback", 2);
        return getNodeListDefault(coin_lower);
    }
    // Get monero nodes from the JSON
    QJsonObject root_obj = json_doc.object(); // {}
    QJsonObject coin_obj = root_obj.value(coin_lower).toObject(); // "monero": {} // "wownero": {}
    QJsonObject clearnet_obj = coin_obj.value("clear").toObject(); // "clear": {} // "onion": {}, "web_compatible": {}
    // Loop through monero nodes (clearnet)
    foreach(const QString& key, clearnet_obj.keys()) {//for (const auto monero_nodes : clearnet_obj) {
        QJsonObject monero_node_obj = clearnet_obj.value(key).toObject();//QJsonObject monero_node_obj = monero_nodes.toObject();
        QVariantMap node_object; // Create an object for each row
        if(key.contains("38081") || key.contains("38089")) { // Temporarily fetch only stagenet nodes (TODO: change to mainnet port upon release)
        node_object.insert("address", key);
        node_object.insert("available", monero_node_obj.value("available").toBool());//std::cout << "available: " << monero_node_obj.value("available").toBool() << "\n";
        ////node_object.insert("", );//////std::cout << ": " << monero_node_obj.value("checks").toArray() << "\n";
        node_object.insert("datetime_checked", monero_node_obj.value("datetime_checked").toString());//std::cout << "datetime_checked: " << monero_node_obj.value("datetime_checked").toString().toStdString() << "\n";
        node_object.insert("datetime_entered", monero_node_obj.value("datetime_entered").toString());//std::cout << "datetime_entered: " << monero_node_obj.value("datetime_entered").toString().toStdString() << "\n";
        node_object.insert("datetime_failed", monero_node_obj.value("datetime_failed").toString());//std::cout << "datetime_failed: " << monero_node_obj.value("datetime_failed").toString().toStdString() << "\n";
        node_object.insert("last_height", monero_node_obj.value("last_height").toInt());//std::cout << "last_height: " << monero_node_obj.value("last_height").toInt() << "\n";
        node_list.append(node_object); // Add node object to the node list
        }
    }
    return node_list;
}
//----------------------------------------------------------------
// Todo: use QProcess to check if monero daemon is running
bool neroshop::Backend::isWalletDaemonRunning() const {
    int monerod = Process::get_process_by_name("monerod");
    if(monerod == -1) { return false; }
    std::cout << "\033[1;90;49m" << "monerod is running (ID:" << monerod << ")\033[0m" << std::endl; 
    return true;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
// TODO: replace function return type with enum
QVariantList neroshop::Backend::validateDisplayName(const QString& display_name) const {
    // username (will appear only in lower-case letters within the app)
    std::string username = display_name.toStdString();
    // Empty display names are acceptable
    if(display_name.isEmpty()) return { true, "" };
    // If display name is set, make sure it is at least 2 characters short (min_user_length=2)
    unsigned int min_user_length = 2;
    if(username.length() < min_user_length) {
        std::string message = "Your username must be at least " + std::to_string(min_user_length) + " characters in length";
        return { false, QString::fromStdString(message) };
    }
    // make sure username is at least 30 characters long (max_user_length=30)
    unsigned int max_user_length = 30; // what if a company has a long name? :o // also consider the textbox's max-length
    if(username.length() > max_user_length) {
        std::string message = "Your username must not exceed " + std::to_string(max_user_length) + " characters in length";
        return { false, QString::fromStdString(message) };
    }
    for(int i = 0; i < username.length(); i++)
    {
        // make sure username does not contain any spaces //std::cout << username[i] << " - char\n";
        if(isspace(username[i])) {
            std::string message = "Your username cannot contain any spaces";
            return { false, QString::fromStdString(message) };
        }
        // make sure username can only contain letters, numbers, and these specific symbols: a hyphen, underscore, and period (-,_,.) //https://stackoverflow.com/questions/39819830/what-are-the-allowed-character-in-snapchat-username#comment97381763_41959421
        // symbols like @,#,$,etc. are invalid
        if(!isalnum(username[i])) {
            if(username[i] != '-'){
            if(username[i] != '_'){
            if(username[i] != '.'){
                std::string message = "Your username cannot contain any symbols except (-, _, .)";// + username[i];
                return { false, QString::fromStdString(message) };
            }}}
        }
    }
    // make sure username begins with a letter (username cannot start with a symbol or number)
    char first_char = username.at(username.find_first_of(username));
    if(!isalpha(first_char)) {
        std::string message = "Your username must begin with a letter";// + first_char;
        return { false, QString::fromStdString(message) };
    }
    // make sure username does not end with a symbol (username can end with a number, but NOT with a symbol)
    char last_char = username.at(username.find_last_of(username));
    if(!isalnum(last_char)) { //if(last_char != '_') { // underscore allowed at end of username? :o
        std::string message = "Your username must end with a letter or number";// + last_char;
        return { false, QString::fromStdString(message) };
    }
    // the name guest is reserved for guests only, so it cannot be used by any other user
    if(neroshop::string::lower(username) == "guest") {
        std::string message = "The name \"Guest\" is reserved for guests ONLY";
        return { false, QString::fromStdString(message) };
    }
    
    // Check database to see if display name is available
    auto name_check_result = checkDisplayName(display_name);
    if(!name_check_result[0].toBool()) {////if(!Validator::validate_display_name(display_name.toStdString())) return false;
        bool boolean_result = name_check_result[0].toBool();
        QString message_result = name_check_result[1].toString();
        return { boolean_result, message_result };
    }    
    return { true, "" };
}
//----------------------------------------------------------------
// TODO: replace function return type with enum
QVariantList neroshop::Backend::checkDisplayName(const QString& display_name) const {    
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database->table_exists("users")) { return {true, ""}; } 
    std::string name = database->get_text_params("SELECT name FROM users WHERE name = $1 COLLATE NOCASE;", { display_name.toStdString() });
    if(name.empty()) return { true, "Display name is available for use" };// Name is not taken which means that the user is good to go!
    // Empty display names are acceptable
    bool is_name_empty = display_name.isEmpty();
    if(is_name_empty) return { true, "No display name set" };
    // Note: both database and input display names are converted to lowercase strings and then compared within the app
    std::string name_lowercase = QString::fromStdString(name).toLower().toStdString();//QString::fromUtf8(name.data(), name.size()).toLower();
    std::string display_name_lowercase = display_name.toLower().toStdString();
    if(name_lowercase == display_name_lowercase) { 
	    return { false, "This username is already taken" };////result_list << false << QString("This username is already taken");return result_list;
	}   
	return { true, "" };
}
//----------------------------------------------------------------
// TODO: replace function return type with enum
QVariantList neroshop::Backend::registerUser(WalletController* wallet_controller, const QString& display_name, UserController * user_controller) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // Validate display name
    auto name_validation_result = validateDisplayName(display_name);
    if(!name_validation_result[0].toBool()) {
        bool boolean_result = name_validation_result[0].toBool();
        QString message_result = name_validation_result[1].toString();
        return { boolean_result, message_result };
    }
    // Get wallet primary address and check its validity
    std::string primary_address = wallet_controller->getPrimaryAddress().toStdString();//neroshop::print("Primary address: \033[1;33m" + primary_address + "\033[1;37m\n");
    if(!wallet_controller->getWallet()->is_valid_address(primary_address)) {
        return { false, "Invalid monero address" };
    }
    // Store login credentials in database
    // Todo: make this command (DB entry) a client request that the server must respond to and the consensus must agree with
    // Note: Multiple users cannot have the same display_name. Each display_name must be unique!
    std::string user_id = database->get_text_params("INSERT INTO users(name, monero_address) VALUES($1, $2) RETURNING monero_address;", { display_name.toStdString(), primary_address });//int user_id = database->get_integer_params("INSERT INTO users(name, monero_address) VALUES($1, $2) RETURNING id;", { display_name.toStdString(), primary_address.toStdString() });
    if(user_id.empty()) { return { false, "Account registration failed (due to database error)" }; }//if(user_id == 0) { return { false, "Account registration failed (due to database error)" }; }
    // Create cart for user
    QString cart_uuid = QUuid::createUuid().toString();
    cart_uuid = cart_uuid.remove("{").remove("}"); // remove brackets
    database->execute_params("INSERT INTO cart (uuid, user_id) VALUES ($1, $2)", { cart_uuid.toStdString(), user_id });
    // initialize user obj (Todo: make a seperate class headers + source files for User)
    std::unique_ptr<neroshop::User> seller(neroshop::Seller::on_login(*wallet_controller->getWallet()));
    user_controller->_user = std::move(seller);
    if (user_controller->getUser() == nullptr) {
        return {false, "user is NULL"};
    }
    emit user_controller->userChanged();
    emit user_controller->userLogged();
    //user_controller->rateItem("3c20978b-a543-4c58-bc68-5f900027796f", 3, "This product is aiight");
    // Display registration message
    neroshop::print(((!display_name.isEmpty()) ? "Welcome to neroshop, " : "Welcome to neroshop") + display_name.toStdString(), 4);
    return { true, "" };
}
//----------------------------------------------------------------
bool neroshop::Backend::loginWithWalletFile(WalletController* wallet_controller, const QString& path, const QString& password, UserController * user_controller) { 
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // Open wallet file
    std::packaged_task<bool(void)> open_wallet_task([wallet_controller, path, password]() -> bool {
        if(!wallet_controller->open(path, password)) {
            ////throw std::runtime_error("Invalid password or wallet network type");
            return false;
        }
        return true;
    });
    std::future<bool> future_result = open_wallet_task.get_future();
    // move the task (function) to a separate thread to prevent blocking of the main thread
    std::thread worker(std::move(open_wallet_task));
    worker.detach(); // join may block but detach won't
    bool wallet_opened = future_result.get();
    if(!wallet_opened) return false;
    // Get the primary address
    std::string primary_address = wallet_controller->getPrimaryAddress().toStdString();
    // Check database to see if user key (hash of primary address) exists
    bool user_found = database->get_integer_params("SELECT EXISTS(SELECT * FROM users WHERE monero_address = $1)", { primary_address });
    // If user key is not found in the database, then create one. This is like registering for an account
    if(!user_found) {
        // In reality, this function will return false if user key is not registered in the database
        neroshop::print("user not found in database. Please try again or register", 1);
        wallet_controller->close();
        return false;
    }
    // Save user information in memory
    std::string display_name = database->get_text_params("SELECT name FROM users WHERE monero_address = $1", { primary_address });
    std::unique_ptr<neroshop::User> seller(neroshop::Seller::on_login(*wallet_controller->getWallet()));
    user_controller->_user = std::move(seller);
    if(user_controller->getUser() == nullptr) {
        return false;//{false, "user is NULL"};
    }
    emit user_controller->userChanged();
    emit user_controller->userLogged();
    // Display message
    neroshop::print("Welcome back, user " + ((!display_name.empty()) ? (display_name + " (id: " + primary_address + ")") : primary_address), 4);
    return true;
}
//----------------------------------------------------------------
bool neroshop::Backend::loginWithMnemonic(WalletController* wallet_controller, const QString& mnemonic, UserController * user_controller) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // Initialize monero wallet with existing wallet mnemonic
    if(!wallet_controller->restoreFromMnemonic(mnemonic)) {
        throw std::runtime_error("Invalid mnemonic or wallet network type");
        return false;    
    }
    // Get the primary address
    std::string primary_address = wallet_controller->getPrimaryAddress().toStdString();
    // Check database to see if user key (hash of primary address) exists
    bool user_key_found = database->get_integer_params("SELECT EXISTS(SELECT * FROM users WHERE monero_address = $1)", { primary_address });
    // If user key is not found in the database, then create one. This is like registering for an account
    if(!user_key_found) {
        // In reality, this function will return false if user key is not registered in the database
        neroshop::print("user key not found in database. Please try again or register", 1);
        wallet_controller->close();
        return false;
    }
    // Save user information in memory
    int user_id = database->get_integer_params("SELECT id FROM users WHERE monero_address = $1", { primary_address });
    // Display message
    std::string display_name = database->get_text_params("SELECT name FROM users WHERE monero_address = $1", { primary_address });
    neroshop::print("Welcome back, user " + ((!display_name.empty()) ? (display_name + " (id: " + primary_address + ")") : primary_address), 4);
    return true;
}
//----------------------------------------------------------------
bool neroshop::Backend::loginWithKeys(WalletController* wallet_controller, UserController * user_controller) {
/*
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // Get the wallet from the wallet controller
    neroshop::Wallet * wallet = wallet_controller->getWallet();
    // Initialize monero wallet with existing wallet mnemonic
    std::string primary_address;
    std::string secret_view_key;
    std::string secret_spend_key;
    std::cout << "Please enter your primary address:\n";
    std::getline(std::cin, primary_address);
    std::cout << "Please enter your secret view key:\n";
    std::getline(std::cin, secret_view_key);
    std::cout << "Please enter your secret spend key (optional):\n";
    std::getline(std::cin, secret_spend_key);
    // todo: allow user to specify a custom location for the wallet keyfile or use a default location
    wallet_controller->restoreFromKeys(primary_address, secret_view_key, secret_spend_key);
    // Get the hash of the primary address
    std::string user_auth_key;// = neroshop::algo::sha256(primary_address);
    Validator::generate_sha256_hash(primary_address, user_auth_key); // temp
    neroshop::print("Primary address: \033[1;33m" + primary_address + "\033[1;37m\nSHA256 hash: " + user_auth_key);
    //$ echo -n "528qdm2pXnYYesCy5VdmBneWeaSZutEijFVAKjpVHeVd4unsCSM55CjgViQsK9WFNHK1eZgcCuZ3fRqYpzKDokqSKp4yp38" | sha256sum
    // Check database to see if user key (hash of primary address) exists
    bool user_key_found = database->get_integer_params("SELECT EXISTS(SELECT * FROM users WHERE key = $1)", { user_auth_key });
    // If user key is not found in the database, then create one. This is like registering for an account
    if(!user_key_found) {
        // In reality, this function will return false if user key is not registered in the database
        neroshop::print("user key not found in database. Please try again or register", 1);
        wallet_controller->close();
        return false;
    }
    // Save user information in memory
    int user_id = database->get_integer_params("SELECT id FROM users WHERE key = $1", { user_auth_key });
    // Display message
    std::string display_name = database->get_text_params("SELECT name FROM users WHERE monero_address = $1", { primary_address });
    neroshop::print("Welcome back, user " + ((!display_name.empty()) ? (display_name + " (id: " + primary_address + ")") : primary_address), 4);
    return true;
*/
    return false;
}
//----------------------------------------------------------------
bool neroshop::Backend::loginWithHW(WalletController* wallet_controller, UserController * user_controller) {
    return false;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
void neroshop::Backend::startServerDaemon() {
    #ifdef Q_OS_WIN
    QString program = "neromon.exe";
    #else
    QString program = "./neromon";
    #endif

    // Note: If the calling process exits, the detached process will continue to run unaffected.
    qint64 pid = -1;
    bool success = QProcess::startDetached(program, {}, QString(), &pid);
    if(!success) { throw std::runtime_error("neroshop daemon process could not be started"); }
    std::cout << "\033[35mneromon started (pid: " << pid << ")\033[0m\n";
}
//----------------------------------------------------------------
void neroshop::Backend::waitForServerDaemon() {
    ::sleep(2);
}
//----------------------------------------------------------------
void neroshop::Backend::connectToServerDaemon() {
    // We will use JSON to write the request data that will sent to the server
    neroshop::Client * client = neroshop::Client::get_main_client();
	int client_port = 40441;
	std::string client_ip = "0.0.0.0";//"localhost";//0.0.0.0 means anyone can connect to your server
	if(!client->connect(client_port, client_ip)) {
	    // free process
	    ////delete server_process; // kills process
	    ////server_process = nullptr;
	    // exit application
	    exit(0);
	} else std::cout << client->read() << std::endl;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
#include <nlohmann/json.hpp>
#include "../core/rpc.hpp"
void neroshop::Backend::testWriteJson() {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    /*QJsonObject jsonObject; // base Object (required)
    
    //QJsonObject json_attributes_object; // subObject (optional)
    
    QJsonArray json_color_array; // An array of colors
    json_color_array.insert(json_color_array.size(), QJsonValue("red"));
    json_color_array.insert(json_color_array.size(), QJsonValue("green"));
    json_color_array.insert(json_color_array.size(), QJsonValue("blue"));
    
    QJsonArray json_size_array; // An array of sizes
    json_size_array.insert(json_size_array.size(), QJsonValue("XS"));
    json_size_array.insert(json_size_array.size(), QJsonValue("S"));
    json_size_array.insert(json_size_array.size(), QJsonValue("M"));
    json_size_array.insert(json_size_array.size(), QJsonValue("L"));
    json_size_array.insert(json_size_array.size(), QJsonValue("XL"));
    
    //jsonObject.insert
    jsonObject.insert(QString("weight"), QJsonValue(12.5));
    jsonObject.insert(QString("color"), json_color_array);
    jsonObject.insert(QString("size"), json_size_array);
    
    //jsonObject.insert("attributes", json_attributes_object);
    
    // Convert JSON to QString
    QJsonDocument doc(jsonObject);
    QString strJson = doc.toJson(QJsonDocument::Compact); // https://doc.qt.io/qt-6/qjsondocument.html#JsonFormat-enum
    // Display JSON as string
    std::cout << strJson.toStdString() << std::endl;*/
}
//----------------------------------------------------------------
void neroshop::Backend::testfts5() {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    database->execute("CREATE VIRTUAL TABLE email USING fts5(sender, title, body);");
}
//----------------------------------------------------------------
//----------------------------------------------------------------

