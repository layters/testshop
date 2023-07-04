#include "backend.hpp"

#include <QClipboard>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess> // Note: QProcess is not supported on VxWorks, iOS, tvOS, or watchOS.
#include <QUuid>

#include "../neroshop_config.hpp"
#include "../core/version.hpp"
#include "../core/protocol/p2p/serializer.hpp"
#include "daemon_manager.hpp"
#include "../core/cart.hpp"
#include "../core/protocol/transport/client.hpp"
#include "../core/price/currency_converter.hpp" // neroshop::Converter::is_supported_currency
#include "../core/price/currency_map.hpp"
#include "../core/crypto/sha256.hpp" // sha256
#include "../core/database/database.hpp"
#include "../core/tools/script.hpp"
#include "../core/settings.hpp"
#include "script_controller.hpp" // neroshop::Script::get_table_string
#include "../core/tools/tools.hpp"
#include "../core/tools/logger.hpp"
#include "../core/tools/process.hpp"
#include "../core/category.hpp"
#include "../core/tools/regex.hpp"
#include "../core/crypto/rsa.hpp"
#include "../core/protocol/p2p/mapper.hpp"

#include <future>
#include <thread>

namespace neroshop_filesystem = neroshop::filesystem;

neroshop::Backend::Backend(QObject *parent) : QObject(parent) {}

neroshop::Backend::~Backend() {
    #ifdef NEROSHOP_DEBUG
    std::cout << "backend deleted\n";
    #endif
}
//----------------------------------------------------------------
//----------------------------------------------------------------
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
        database->execute("ALTER TABLE cart_item ADD COLUMN seller_id TEXT REFERENCES users(monero_address);"); // for a multi-vendor cart, specifying the seller_id is important!
        //database->execute("ALTER TABLE cart_item ADD COLUMN item_price numeric;"); // sales_price will be used for the final pricing rather than the retail_price
        //database->execute("ALTER TABLE cart_item ADD COLUMN item_weight REAL;");//database->execute("CREATE TABLE cart_item(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, cart_id TEXT REFERENCES cart(id), product_id TEXT REFERENCES products(id), item_qty INTEGER, item_price NUMERIC, item_weight REAL);");
        database->execute("CREATE UNIQUE INDEX index_cart_item ON cart_item (cart_id, product_id);"); // cart_id and product_id duo MUST be unqiue for each row
    }
    // orders (purchase_orders)
    if(!database->table_exists("orders")) { // TODO: rename to order_requests or nah?
        database->execute("CREATE TABLE orders(uuid TEXT NOT NULL PRIMARY KEY);");//database->execute("ALTER TABLE orders ADD COLUMN ?col ?datatype;");
        database->execute("ALTER TABLE orders ADD COLUMN created_at TEXT DEFAULT CURRENT_TIMESTAMP;"); // creation_date // to get UTC time: set to datetime('now');
        //database->execute("ALTER TABLE orders ADD COLUMN number TEXT;"); // uuid
        database->execute("ALTER TABLE orders ADD COLUMN status TEXT;");
        database->execute("ALTER TABLE orders ADD COLUMN customer_id TEXT REFERENCES users(monero_address);"); // the user that placed the order
        // Data below this comment will be stored in order_data as JSON TEXT
        //database->execute("ALTER TABLE orders ADD COLUMN weight REAL;"); // weight of all order items combined - not essential
        database->execute("ALTER TABLE orders ADD COLUMN subtotal INTEGER;");
        database->execute("ALTER TABLE orders ADD COLUMN discount INTEGER;");
        //database->execute("ALTER TABLE orders ADD COLUMN shipping_method TEXT;");
        database->execute("ALTER TABLE orders ADD COLUMN shipping_cost INTEGER;");
        database->execute("ALTER TABLE orders ADD COLUMN total INTEGER;");
        database->execute("ALTER TABLE orders ADD COLUMN payment_option TEXT;"); // escrow (2 of 3), multisig (2 of 2), finalize (no escrow)
        database->execute("ALTER TABLE orders ADD COLUMN coin TEXT;"); // monero, wownero
        database->execute("ALTER TABLE orders ADD COLUMN notes TEXT;"); // additional message for seller
        //database->execute("ALTER TABLE orders ADD COLUMN order_data TEXT;"); // encrypted JSON
        // order_item
        // TODO: remove order_item table and replace it with order_data JSON column
        database->execute("CREATE TABLE order_item(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
        "order_id TEXT REFERENCES orders(uuid) ON DELETE CASCADE, "
        "product_id TEXT REFERENCES products(uuid), "
        "seller_id TEXT REFERENCES users(monero_address), "
        "quantity INTEGER"
        ");");
        //database->execute("ALTER TABLE order_item ADD COLUMN unit_price ?datatype;");
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
    if(!database->table_exists("mappings")) { 
        database->execute("CREATE VIRTUAL TABLE mappings USING fts5(search_term, key, content, tokenize='porter unicode61');");
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
    std::string sha256sum = neroshop::crypto::sha256(db_content.str());
    std::cout << "sha256sum (data.sqlite3): " << sha256sum << std::endl;
    return sha256sum; // database may have to be closed first in order to get the accurate hash
}
//----------------------------------------------------------------
//----------------------------------------------------------------
QVariantList neroshop::Backend::getCategoryList(bool sort_alphabetically) const {
    QVariantList category_list;
    
    for (const auto& category : predefined_categories) {
        QVariantMap category_object;
        category_object.insert("id", category.id);
        category_object.insert("name", QString::fromStdString(category.name));
        category_object.insert("description", QString::fromStdString(category.description));
        category_object.insert("thumbnail", QString::fromStdString(category.thumbnail));
        category_list.append(category_object);
    }
    
    if(sort_alphabetically == true) {
        std::sort(category_list.begin(), category_list.end(), [](const QVariant& a, const QVariant& b) {
            return a.toMap()["name"].toString().compare(b.toMap()["name"].toString(), Qt::CaseInsensitive) < 0;
        });
    }

    return category_list;
}

//----------------------------------------------------------------
int neroshop::Backend::getCategoryIdByName(const QString& category_name) const {
    return get_category_id_by_name(category_name.toStdString());
}
//----------------------------------------------------------------
int neroshop::Backend::getCategoryProductCount(int category_id) const {
    db::Sqlite3 * database = neroshop::get_database();

    std::string query = "SELECT COUNT(*) FROM (SELECT DISTINCT search_term, key FROM mappings WHERE search_term MATCH ?";//"SELECT COUNT(*) FROM (SELECT DISTINCT search_term, key FROM mappings WHERE search_term MATCH ?);";
    std::string category = get_category_name_by_id(category_id);
    
    // Replace ampersands with wildcard (*)
    std::replace(category.begin(), category.end(), '&', '*');
    // Add double quotes around the category for phrase matching
    category = "\"" + category + "\"";

    query += ")";

    int category_product_count = database->get_integer_params(query, { category });
    return category_product_count;
}
//----------------------------------------------------------------
bool neroshop::Backend::createFolders() {
    // Save the image file to a specific location
    std::string config_path = NEROSHOP_DEFAULT_CONFIGURATION_PATH;
    std::string cache_folder = config_path + "/" + NEROSHOP_CACHE_FOLDER_NAME;
    
    // /datastore/
    if(!neroshop_filesystem::is_directory(cache_folder)) {
        neroshop::print("Creating directory \"" + cache_folder + "\" (^_^) ...", 2);
        if(!neroshop_filesystem::make_directory(cache_folder)) {
            neroshop::print("Failed to create folder \"" + cache_folder + "\" (ᵕ人ᵕ)!", 1);
            return false;
        }
        neroshop::print("\033[1;97;49mcreated path \"" + cache_folder + "\"");
    }    
    //--------------------------------
    std::string products_folder = cache_folder + "/" + NEROSHOP_CATALOG_FOLDER_NAME;
    // folder with the name <listing_id> should contain all product images for particular listing

    // datastore/listings/
    if (!neroshop_filesystem::is_directory(products_folder)) {
        neroshop::print("Creating directory \"" + products_folder + "\" (^_^) ...", 2);
        if (!neroshop_filesystem::make_directory(products_folder)) {
            neroshop::print("Failed to create folder \"" + products_folder + "\" (ᵕ人ᵕ)!", 1);
            return false;
        }
        neroshop::print("\033[1;97;49mcreated path \"" + products_folder + "\"");
    }
    //--------------------------------
    // TODO: uncomment this
    /*std::string avatars_folder = cache_folder + "/" + NEROSHOP_AVATAR_FOLDER_NAME;
    
    // datastore/avatars/
    if (!neroshop_filesystem::is_directory(avatars_folder)) {
        neroshop::print("Creating directory \"" + avatars_folder + "\" (^_^) ...", 2);
        if (!neroshop_filesystem::make_directory(avatars_folder)) {
            neroshop::print("Failed to create folder \"" + avatars_folder + "\" (ᵕ人ᵕ)!", 1);
            return false;
        }
        neroshop::print("\033[1;97;49mcreated path \"" + avatars_folder + "\"");
    }*/
    //--------------------------------
    return true;
}
//----------------------------------------------------------------
#include <QImage>
#include <QPixmap>
//----------------------------------------------------------------
bool neroshop::Backend::saveProductImage(const QString& fileName, const QString& listingKey) {
    std::string config_path = NEROSHOP_DEFAULT_CONFIGURATION_PATH;
    std::string cache_folder = config_path + "/" + NEROSHOP_CACHE_FOLDER_NAME;
    std::string listings_folder = cache_folder + "/" + NEROSHOP_CATALOG_FOLDER_NAME;
    //----------------------------------------
    std::string image_file = fileName.toStdString(); // Full path with file name
    std::string image_name = image_file.substr(image_file.find_last_of("\\/") + 1);// get filename from path (complete base name)
    //----------------------------------------    
    // datastore/listings/<listing_key>
    std::string products_folder = listings_folder + "/" + listingKey.toStdString();
    if (!neroshop_filesystem::is_directory(products_folder)) {
        neroshop::print("Creating directory \"" + products_folder + "\" (^_^) ...", 2);
        if (!neroshop_filesystem::make_directory(products_folder)) {
            neroshop::print("Failed to create folder \"" + products_folder + "\" (ᵕ人ᵕ)!", 1);
            return false;
        }
        neroshop::print("\033[1;97;49mcreated path \"" + products_folder + "\"");
    }
    //----------------------------------------
    // Generate the final destination path
    std::string destinationPath = products_folder + "/" + image_name;
    // Check if image already exists in cache so that we do not export the same image more than once
    if(!neroshop_filesystem::is_file(destinationPath)) {
        // Image Loader crashes when image resolution is too large (ex. 4096 pixels wide) so we need to scale it!!
        QImage sourceImage;
        sourceImage.load(fileName);
        QSize imageSize = sourceImage.size();
        int maxWidth = 1200; // Set the maximum width for the resized image
        int maxHeight = 1200; // Set the maximum height for the resized image

        // Check if the image size is smaller than the maximum size
        if (imageSize.width() <= maxWidth && imageSize.height() <= maxHeight) {
            // Keep the original image since it's already within the size limits
        } else {
            // Calculate the new size while maintaining the aspect ratio
            QSize newSize = imageSize.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio);

            // Resize the image if it exceeds the maximum dimensions
            if (imageSize != newSize) {
                sourceImage = sourceImage.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
        }

        // Convert the QImage to QPixmap for further processing or saving
        QPixmap resizedPixmap = QPixmap::fromImage(sourceImage);

        // Save the resized image
        resizedPixmap.save(QString::fromStdString(destinationPath));
    }
    neroshop::print("exported \"" + fileName.toStdString() + "\" to \"" + cache_folder + "\"", 3);
    return true;
}
//----------------------------------------------------------------
QVariantMap neroshop::Backend::uploadProductImage(const QString& fileName, int image_id) {
    QVariantMap image;
    //----------------------------------------
    // Read image from file and retrieve its contents
    std::ifstream product_image_file(fileName.toStdString(), std::ios::binary); // std::ios::binary is the same as std::ifstream::binary
    if(!product_image_file.good()) {
        std::cout << NEROSHOP_TAG "failed to load " << fileName.toStdString() << std::endl; 
        return {};
    }
    product_image_file.seekg(0, std::ios::end);
    size_t size = static_cast<int>(product_image_file.tellg()); // in bytes
    // Limit product image size to 12582912 bytes (12 megabyte)
    // Todo: Database cannot scale to billions of users if I am storing blobs so I'll have to switch to text later
    const int max_bytes = 12582912;
    double kilobytes = max_bytes / 1024.0;
    double megabytes = kilobytes / 1024.0;
    if(size >= max_bytes) {
        neroshop::print("Product upload image cannot exceed " + std::to_string(megabytes) + " MB (twelve megabyte)", 1);
        return {};
    }
    product_image_file.seekg(0);
    std::vector<unsigned char> buffer(size);
    if(!product_image_file.read(reinterpret_cast<char *>(&buffer[0]), size)) {
        std::cout << NEROSHOP_TAG "error: only " << product_image_file.gcount() << " could be read";
        return {}; // exit function
    }
    product_image_file.close();
    //----------------------------------------
    if(!createFolders()) {
        return {};
    }
    //----------------------------------------
    // Create the image VariantMap (object)
    std::string image_file = fileName.toStdString(); // Full path with file name
    std::string image_name = image_file.substr(image_file.find_last_of("\\/") + 1);
    image["name"] = QString::fromStdString(image_name);//fileName;
    qint64 imageSize64 = static_cast<qint64>(size);
    image["size"] = QVariant::fromValue(imageSize64);
    image["id"] = image_id;
    image["source"] = fileName;
    //----------------------------------------
    return image;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
int neroshop::Backend::getProductStarCount(const QString& product_id) {
    // Get total number of star ratings for a specific product
    QVariantList product_ratings = getProductRatings(product_id);
    int ratings_count = product_ratings.size();
    return ratings_count;
}
//----------------------------------------------------------------
int neroshop::Backend::getProductStarCount(const QString& product_id, int star_number) {
    // Get total number of N star ratings for a specific product
    if(star_number > 5) star_number = 5;
    if(star_number < 1) star_number = 1;
    int star_count = 0;
    QVariantList product_ratings = getProductRatings(product_id);
    for (const QVariant& variant : product_ratings) {
        QVariantMap rating_obj = variant.toMap();
        int rating_stars = rating_obj["stars"].toInt();
        if (rating_stars == star_number) {
            star_count++;
        }
    }
    return star_count;
}
//----------------------------------------------------------------
float neroshop::Backend::getProductAverageStars(const QString& product_id) {
    // Get number of star ratings for a specific product
    QVariantList product_ratings = getProductRatings(product_id);
    int total_star_ratings = product_ratings.size();
    if(total_star_ratings == 0) return 0.0f;
    // Get number of 1, 2, 3, 4, and 5 star_ratings
    int one_star_count = 0, two_star_count = 0, three_star_count = 0, four_star_count = 0, five_star_count = 0;
    for (const QVariant& variant : product_ratings) {
        QVariantMap rating_obj = variant.toMap();
        int rating_stars = rating_obj["stars"].toInt();
        if (rating_stars == 1) {
            one_star_count++;
        } else if (rating_stars == 2) {
            two_star_count++;
        } else if (rating_stars == 3) {
            three_star_count++;
        } else if (rating_stars == 4) {
            four_star_count++;
        } else if (rating_stars == 5) {
            five_star_count++;
        }
    }
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
QVariantList neroshop::Backend::getProductRatings(const QString& product_id) {
    Client * client = Client::get_main_client();
    //----------------------------------
    std::string command = "SELECT DISTINCT key FROM mappings WHERE search_term = $1 AND content = 'product_rating'";
    db::Sqlite3 * database = neroshop::get_database();
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    // Bind value to parameter arguments
    QByteArray productIdByteArray = product_id.toUtf8();
    if(sqlite3_bind_text(stmt, 1, productIdByteArray.constData(), productIdByteArray.length(), SQLITE_STATIC) != SQLITE_OK) {
        neroshop::print("sqlite3_bind_text: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(stmt);
        return {};
    }
    //----------------------------------
    QVariantList product_ratings;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        for(int i = 0; i < sqlite3_column_count(stmt); i++) { 
            QVariantMap product_rating_obj; // Create object for each key (row)
            
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));//std::cout << column_value  << " (" << i << ")" << std::endl;
            if(column_value == "NULL") continue; // Skip invalid columns
            QString key = QString::fromStdString(column_value);//std::cout << key.toStdString() << "\n";
            
            // Get the value of the corresponding key from the DHT
            std::string response;
            client->get(key.toStdString(), response); // TODO: error handling
            std::cout << "Received response (get): " << response << "\n";
            // Parse the response
            nlohmann::json json = nlohmann::json::parse(response);
            if(json.contains("error")) {
                /*int rescode = database->execute_params("DELETE FROM mappings WHERE key = ?1", { key.toStdString() });
                if(rescode != SQLITE_OK) neroshop::print("sqlite error: DELETE failed", 1);*/
                //emit productRatingsChanged();
                continue; // Key is lost or missing from DHT, skip to next iteration
            }
            
            const auto& response_obj = json["response"];
            assert(response_obj.is_object());
            if (response_obj.contains("value") && response_obj["value"].is_string()) {
                const auto& value = response_obj["value"].get<std::string>();
                nlohmann::json value_obj = nlohmann::json::parse(value);
                assert(value_obj.is_object());//std::cout << value_obj.dump(4) << "\n";
                std::string metadata = value_obj["metadata"].get<std::string>();
                if (metadata != "product_rating") { std::cerr << "Invalid metadata. \"product_rating\" expected, got \"" << metadata << "\" instead\n"; continue; }
                product_rating_obj.insert("key", key);
                product_rating_obj.insert("rater_id", QString::fromStdString(value_obj["rater_id"].get<std::string>()));
                product_rating_obj.insert("comments", QString::fromStdString(value_obj["comments"].get<std::string>()));
                product_rating_obj.insert("signature", QString::fromStdString(value_obj["signature"].get<std::string>()));
                product_rating_obj.insert("stars", value_obj["stars"].get<int>());
            }
            
            product_ratings.append(product_rating_obj);
        }
    }
    //----------------------------------
    return product_ratings;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
int neroshop::Backend::getSellerGoodRatings(const QString& user_id) {
    int good_ratings_count = 0;
    QVariantList seller_ratings = getSellerRatings(user_id);
    // Get seller's good (positive) ratings
    for (const QVariant& variant : seller_ratings) {
        QVariantMap rating_obj = variant.toMap();
        int rating_score = rating_obj["score"].toInt();
        if(rating_score == 1) {
            good_ratings_count++;
        }
    }
    return good_ratings_count;
}
//----------------------------------------------------------------
int neroshop::Backend::getSellerBadRatings(const QString& user_id) {
    int bad_ratings_count = 0;
    QVariantList seller_ratings = getSellerRatings(user_id);
    // Get seller's bad (negative) ratings
    for (const QVariant& variant : seller_ratings) {
        QVariantMap rating_obj = variant.toMap();
        int rating_score = rating_obj["score"].toInt();
        if(rating_score == 0) {
            bad_ratings_count++;
        }
    }    
    return bad_ratings_count;
}
//----------------------------------------------------------------
int neroshop::Backend::getSellerRatingsCount(const QString& user_id) {
    QVariantList seller_ratings = getSellerRatings(user_id);
    int ratings_count = seller_ratings.size();
    return ratings_count;
}
//----------------------------------------------------------------
int neroshop::Backend::getSellerReputation(const QString& user_id) {
    int good_ratings_count = 0, bad_ratings_count = 0;
    QVariantList seller_ratings = getSellerRatings(user_id);
    int ratings_count = seller_ratings.size();
    if(ratings_count <= 0) return 0; // seller has not yet been rated so his or her reputation will be 0%
    // Get seller's good (positive) ratings
    for (const QVariant& variant : seller_ratings) {
        QVariantMap rating_obj = variant.toMap();
        int rating_score = rating_obj["score"].toInt();
        if(rating_score == 1) {
            good_ratings_count++;
        }
    }
    // Calculate seller reputation
    double reputation = (good_ratings_count / static_cast<double>(ratings_count)) * 100;
    return static_cast<int>(reputation); // convert reputation to an integer (for easier readability)
}
//----------------------------------------------------------------
// returns an array of ratings objects
QVariantList neroshop::Backend::getSellerRatings(const QString& user_id) {
    Client * client = Client::get_main_client();
    //----------------------------------
    std::string command = "SELECT DISTINCT key FROM mappings WHERE search_term = $1 AND content = 'seller_rating'";
    db::Sqlite3 * database = neroshop::get_database();
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    // Bind value to parameter arguments
    QByteArray userIdByteArray = user_id.toUtf8();
    if(sqlite3_bind_text(stmt, 1, userIdByteArray.constData(), userIdByteArray.length(), SQLITE_STATIC) != SQLITE_OK) {
        neroshop::print("sqlite3_bind_text: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(stmt);
        return {};
    }
    //----------------------------------
    QVariantList seller_ratings;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        for(int i = 0; i < sqlite3_column_count(stmt); i++) { 
            QVariantMap seller_rating_obj; // Create object for each key (row)
            
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));//std::cout << column_value  << " (" << i << ")" << std::endl;
            if(column_value == "NULL") continue; // Skip invalid columns
            QString key = QString::fromStdString(column_value);//std::cout << key.toStdString() << "\n";
            
            // Get the value of the corresponding key from the DHT
            std::string response;
            client->get(key.toStdString(), response); // TODO: error handling
            std::cout << "Received response (get): " << response << "\n";
            // Parse the response
            nlohmann::json json = nlohmann::json::parse(response);
            if(json.contains("error")) {
                /*int rescode = database->execute_params("DELETE FROM mappings WHERE key = ?1", { key.toStdString() });
                if(rescode != SQLITE_OK) neroshop::print("sqlite error: DELETE failed", 1);*/
                //emit sellerRatingsChanged();
                continue; // Key is lost or missing from DHT, skip to next iteration
            }
            
            const auto& response_obj = json["response"];
            assert(response_obj.is_object());
            if (response_obj.contains("value") && response_obj["value"].is_string()) {
                const auto& value = response_obj["value"].get<std::string>();
                nlohmann::json value_obj = nlohmann::json::parse(value);
                assert(value_obj.is_object());//std::cout << value_obj.dump(4) << "\n";
                std::string metadata = value_obj["metadata"].get<std::string>();
                if (metadata != "seller_rating") { std::cerr << "Invalid metadata. \"seller_rating\" expected, got \"" << metadata << "\" instead\n"; continue; }
                seller_rating_obj.insert("key", key);
                seller_rating_obj.insert("rater_id", QString::fromStdString(value_obj["rater_id"].get<std::string>()));
                seller_rating_obj.insert("comments", QString::fromStdString(value_obj["comments"].get<std::string>()));
                seller_rating_obj.insert("signature", QString::fromStdString(value_obj["signature"].get<std::string>()));
                seller_rating_obj.insert("score", value_obj["score"].get<int>());
            }
            
            seller_ratings.append(seller_rating_obj);
        }
    }
    //----------------------------------
    return seller_ratings;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
QString neroshop::Backend::getDisplayNameByUserId(const QString& user_id) {
    db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    std::string display_name = database->get_text_params("SELECT name FROM users WHERE monero_address = $1", { user_id.toStdString() });
    return QString::fromStdString(display_name);
}

// un-tested
QString neroshop::Backend::getKeyByUserId(const QString& user_id) {
    db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    std::string display_name = database->get_text_params("SELECT key FROM mappings WHERE search_term = $1 AND content = 'account' LIMIT 1;", { user_id.toStdString() });
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
    db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    int quantity = database->get_integer_params("SELECT quantity FROM listings WHERE product_id = $1 AND quantity > 0", { product_id.toStdString() });
    return quantity;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
QVariantList neroshop::Backend::getInventory(const QString& user_id) {
    db::Sqlite3 * database = neroshop::get_database();
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
//----------------------------------------------------------------
//----------------------------------------------------------------
QVariantList neroshop::Backend::getSearchResults(const QString& searchTerm, int count) {
    // Transition from Sqlite to DHT:
    Client * client = Client::get_main_client();
    db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    std::string command = "SELECT DISTINCT key FROM mappings WHERE (search_term MATCH ?1 OR search_term MATCH ?1 || '*') AND (content = 'listing') LIMIT ?2;";//"SELECT DISTINCT key FROM mappings WHERE (search_term MATCH ? OR search_term LIKE '%' || ? || '%' COLLATE NOCASE) AND (content MATCH 'listing');";//"SELECT DISTINCT key FROM mappings WHERE search_term MATCH ? AND content MATCH 'listing';";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    //-------------------------------------------------------
    // Bind value to parameter arguments
    QByteArray searchTermByteArray = searchTerm.toUtf8(); // Convert QString to std::string equivalent
    if(sqlite3_bind_text(stmt, 1, searchTermByteArray.constData(), searchTermByteArray.length(), SQLITE_STATIC) != SQLITE_OK) {
        neroshop::print("sqlite3_bind_text: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(stmt);
        return {};//database->execute("ROLLBACK;"); return {};
    }        
    
    if(sqlite3_bind_int(stmt, 2, count) != SQLITE_OK) {
        neroshop::print("sqlite3_bind_int: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(stmt);
        return {};//database->execute("ROLLBACK;"); return {};
    }            
    //-------------------------------------------------------
    // Check whether the prepared statement returns no data (for example an UPDATE)
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::print("No data found. Be sure to use an appropriate SELECT statement", 1);
        return {};
    }
    
    QVariantList catalog;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            QVariantMap listing; // Create an object for each row
            QVariantList product_images;
            
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));//std::cout << column_value  << " (" << i << ")" << std::endl;
            QString key = QString::fromStdString(column_value);
            // Get the value of the corresponding key from the DHT
            std::string response;
            client->get(key.toStdString(), response); // TODO: error handling
            std::cout << "Received response (get): " << response << "\n";
            // Parse the response
            nlohmann::json json = nlohmann::json::parse(response);
            if(json.contains("error")) { 
                int rescode = database->execute_params("DELETE FROM mappings WHERE key = ?1", { key.toStdString() });
                if(rescode != SQLITE_OK) neroshop::print("sqlite error: DELETE failed", 1);
                //emit categoryProductCountChanged();//(category_id);
                //emit searchResultsChanged();
                continue; // Key is lost or missing from DHT, skip to next iteration
            }
            
            const auto& response_obj = json["response"];
            assert(response_obj.is_object());
            if (response_obj.contains("value") && response_obj["value"].is_string()) {
                const auto& value = response_obj["value"].get<std::string>();
                nlohmann::json value_obj = nlohmann::json::parse(value);
                assert(value_obj.is_object());//std::cout << value_obj.dump(4) << "\n";
                listing.insert("key", key);
                listing.insert("listing_uuid", QString::fromStdString(value_obj["id"].get<std::string>()));
                listing.insert("seller_id", QString::fromStdString(value_obj["seller_id"].get<std::string>()));
                listing.insert("quantity", value_obj["quantity"].get<int>());
                listing.insert("price", value_obj["price"].get<double>());
                listing.insert("currency", QString::fromStdString(value_obj["currency"].get<std::string>()));
                listing.insert("condition", QString::fromStdString(value_obj["condition"].get<std::string>()));
                if(value_obj.contains("location") && value_obj["location"].is_string()) {
                    listing.insert("location", QString::fromStdString(value_obj["location"].get<std::string>()));
                }
                listing.insert("date", QString::fromStdString(value_obj["date"].get<std::string>()));
                assert(value_obj["product"].is_object());
                const auto& product_obj = value_obj["product"];
                listing.insert("product_uuid", QString::fromStdString(product_obj["id"].get<std::string>()));
                listing.insert("product_name", QString::fromStdString(product_obj["name"].get<std::string>()));
                listing.insert("product_description", QString::fromStdString(product_obj["description"].get<std::string>()));
                listing.insert("product_category_id", get_category_id_by_name(product_obj["category"].get<std::string>()));
                //listing.insert("", QString::fromStdString(product_obj[""].get<std::string>()));
                //listing.insert("", QString::fromStdString(product_obj[""].get<std::string>()));
                if (product_obj.contains("images") && product_obj["images"].is_array()) {
                    const auto& images_array = product_obj["images"];
                    for (const auto& image : images_array) {
                        if (image.contains("name") && image.contains("id")) {
                            const auto& image_name = image["name"].get<std::string>();
                            const auto& image_id = image["id"].get<int>();
                            
                            QVariantMap image_map;
                            image_map.insert("name", QString::fromStdString(image_name));
                            image_map.insert("id", image_id);
                            product_images.append(image_map);
                        }
                    }
                    listing.insert("product_images", product_images);
                }
            }
            // Append to catalog only if the key was found successfully
            catalog.append(listing);
        }
    }
    
    sqlite3_finalize(stmt);

    return catalog;
}
//----------------------------------------------------------------
QVariantList neroshop::Backend::getListings() {
    // Transition from Sqlite to DHT:
    Client * client = Client::get_main_client();
    db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    std::string command = "SELECT DISTINCT key FROM mappings WHERE content MATCH 'listing';";
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
    
    QVariantList catalog;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        QVariantMap listing; // Create an object for each row
        QVariantList product_images;

        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));//std::cout << column_value  << " (" << i << ")" << std::endl;
            QString key = QString::fromStdString(column_value);
            // Get the value of the corresponding key from the DHT
            std::string response;
            client->get(key.toStdString(), response); // TODO: error handling
            std::cout << "Received response (get): " << response << "\n";
            // Parse the response
            nlohmann::json json = nlohmann::json::parse(response);
            if(json.contains("error")) {
                int rescode = database->execute_params("DELETE FROM mappings WHERE key = ?1", { key.toStdString() });
                if(rescode != SQLITE_OK) neroshop::print("sqlite error: DELETE failed", 1);
                //emit categoryProductCountChanged();//(category_id);
                //emit searchResultsChanged();
                continue; // Key is lost or missing from DHT, skip to next iteration
            }
            
            const auto& response_obj = json["response"];
            assert(response_obj.is_object());
            if (response_obj.contains("value") && response_obj["value"].is_string()) {
                const auto& value = response_obj["value"].get<std::string>();
                nlohmann::json value_obj = nlohmann::json::parse(value);
                assert(value_obj.is_object());//std::cout << value_obj.dump(4) << "\n";
                listing.insert("key", key);
                listing.insert("listing_uuid", QString::fromStdString(value_obj["id"].get<std::string>()));
                listing.insert("seller_id", QString::fromStdString(value_obj["seller_id"].get<std::string>()));
                listing.insert("quantity", value_obj["quantity"].get<int>());
                listing.insert("price", value_obj["price"].get<double>());
                listing.insert("currency", QString::fromStdString(value_obj["currency"].get<std::string>()));
                listing.insert("condition", QString::fromStdString(value_obj["condition"].get<std::string>()));
                if(value_obj.contains("location") && value_obj["location"].is_string()) {
                    listing.insert("location", QString::fromStdString(value_obj["location"].get<std::string>()));
                }
                listing.insert("date", QString::fromStdString(value_obj["date"].get<std::string>()));
                assert(value_obj["product"].is_object());
                const auto& product_obj = value_obj["product"];
                listing.insert("product_uuid", QString::fromStdString(product_obj["id"].get<std::string>()));
                listing.insert("product_name", QString::fromStdString(product_obj["name"].get<std::string>()));
                listing.insert("product_description", QString::fromStdString(product_obj["description"].get<std::string>()));
                listing.insert("product_category_id", get_category_id_by_name(product_obj["category"].get<std::string>()));
                //listing.insert("", QString::fromStdString(product_obj[""].get<std::string>()));
                //listing.insert("", QString::fromStdString(product_obj[""].get<std::string>()));
                if (product_obj.contains("images") && product_obj["images"].is_array()) {
                    const auto& images_array = product_obj["images"];
                    for (const auto& image : images_array) {
                        if (image.contains("name") && image.contains("id")) {
                            const auto& image_name = image["name"].get<std::string>();
                            const auto& image_id = image["id"].get<int>();
                            
                            QVariantMap image_map;
                            image_map.insert("name", QString::fromStdString(image_name));
                            image_map.insert("id", image_id);
                            product_images.append(image_map);
                        }
                    }
                    listing.insert("product_images", product_images);
                }
            }
            catalog.append(listing);
        }
    }
    
    sqlite3_finalize(stmt);

    return catalog;    
}
//----------------------------------------------------------------
QVariantList neroshop::Backend::getListingsByCategory(int category_id) {
    // Transition from Sqlite to DHT:
    Client * client = Client::get_main_client();
    db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    std::string command = "SELECT DISTINCT key FROM mappings WHERE search_term MATCH ? AND content MATCH 'listing';";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    //-------------------------------------------------------
    std::string category = get_category_name_by_id(category_id);
    
    // Replace ampersands with wildcard (*)
    std::replace(category.begin(), category.end(), '&', '*');
    // Add double quotes around the category for phrase matching
    category = "\"" + category + "\"";
    //-------------------------------------------------------
    // Bind value to parameter arguments
    if(sqlite3_bind_text(stmt, 1, category.c_str(), category.length(), SQLITE_STATIC) != SQLITE_OK) {
        neroshop::print("sqlite3_bind_text: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(stmt);
        return {};//database->execute("ROLLBACK;"); return {};
    }        
    //-------------------------------------------------------
    // Check whether the prepared statement returns no data (for example an UPDATE)
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::print("No data found. Be sure to use an appropriate SELECT statement", 1);
        return {};
    }
    
    QVariantList catalog;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        QVariantMap listing; // Create an object for each row
        QVariantList product_images;

        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));//std::cout << column_value  << " (" << i << ")" << std::endl;
            QString key = QString::fromStdString(column_value);
            // Get the value of the corresponding key from the DHT
            std::string response;
            client->get(key.toStdString(), response); // TODO: error handling
            std::cout << "Received response (get): " << response << "\n";
            // Parse the response
            nlohmann::json json = nlohmann::json::parse(response);
            if(json.contains("error")) {
                int rescode = database->execute_params("DELETE FROM mappings WHERE key = ?1", { key.toStdString() });
                if(rescode != SQLITE_OK) neroshop::print("sqlite error: DELETE failed", 1);
                //emit categoryProductCountChanged();//(category_id);
                //emit searchResultsChanged();
                continue; // Key is lost or missing from DHT, skip to next iteration
            }
            
            const auto& response_obj = json["response"];
            assert(response_obj.is_object());
            if (response_obj.contains("value") && response_obj["value"].is_string()) {
                const auto& value = response_obj["value"].get<std::string>();
                nlohmann::json value_obj = nlohmann::json::parse(value);
                assert(value_obj.is_object());//std::cout << value_obj.dump(4) << "\n";
                listing.insert("key", key);
                listing.insert("listing_uuid", QString::fromStdString(value_obj["id"].get<std::string>()));
                listing.insert("seller_id", QString::fromStdString(value_obj["seller_id"].get<std::string>()));
                listing.insert("quantity", value_obj["quantity"].get<int>());
                listing.insert("price", value_obj["price"].get<double>());
                listing.insert("currency", QString::fromStdString(value_obj["currency"].get<std::string>()));
                listing.insert("condition", QString::fromStdString(value_obj["condition"].get<std::string>()));
                if(value_obj.contains("location") && value_obj["location"].is_string()) {
                    listing.insert("location", QString::fromStdString(value_obj["location"].get<std::string>()));
                }
                listing.insert("date", QString::fromStdString(value_obj["date"].get<std::string>()));
                assert(value_obj["product"].is_object());
                const auto& product_obj = value_obj["product"];
                listing.insert("product_uuid", QString::fromStdString(product_obj["id"].get<std::string>()));
                listing.insert("product_name", QString::fromStdString(product_obj["name"].get<std::string>()));
                listing.insert("product_description", QString::fromStdString(product_obj["description"].get<std::string>()));
                listing.insert("product_category_id", get_category_id_by_name(product_obj["category"].get<std::string>()));
                //listing.insert("weight", QString::fromStdString(product_obj[""].get<std::string>()));
                //listing.insert("other_attr", QString::fromStdString(product_obj[""].get<std::string>()));
                //listing.insert("code", QString::fromStdString(product_obj[""].get<std::string>()));
                //listing.insert("tags", QString::fromStdString(product_obj[""].get<std::string>()));
                if (product_obj.contains("images") && product_obj["images"].is_array()) {
                    const auto& images_array = product_obj["images"];
                    for (const auto& image : images_array) {
                        if (image.contains("name") && image.contains("id")) {
                            const auto& image_name = image["name"].get<std::string>();
                            const auto& image_id = image["id"].get<int>();//source,data, etc.
                            
                            QVariantMap image_map;
                            image_map.insert("name", QString::fromStdString(image_name));
                            image_map.insert("id", image_id);
                            product_images.append(image_map);
                        }
                    }
                    listing.insert("product_images", product_images);
                }
            }
            catalog.append(listing);
        }
    }
    
    sqlite3_finalize(stmt);

    return catalog;
}
//----------------------------------------------------------------
QVariantList neroshop::Backend::getListingsByMostRecent() {
    db::Sqlite3 * database = neroshop::get_database();
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
    db::Sqlite3 * database = neroshop::get_database();
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
    db::Sqlite3 * database = neroshop::get_database();
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
    db::Sqlite3 * database = neroshop::get_database();
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
    db::Sqlite3 * database = neroshop::get_database();
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
    db::Sqlite3 * database = neroshop::get_database();
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
void neroshop::Backend::createOrder(UserController * user_controller, const QString& shipping_address) {
    user_controller->createOrder(shipping_address);
}
//----------------------------------------------------------------
// TODO: run this function periodically
int neroshop::Backend::deleteExpiredOrders() {
    db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // If order is at least 2 years old or older, then it is considered expired. Therefore it must be deleted
    std::string modifier = "+" + std::to_string(2) + " years";//std::cout << modifier << std::endl;
    std::string command = "DELETE FROM orders WHERE datetime(created_at, $1) <= datetime('now');";
    return database->execute_params(command, { modifier }); // 0 = success
}
//----------------------------------------------------------------
//----------------------------------------------------------------
QVariantList neroshop::Backend::getNodeListDefault(const QString& coin) const {
    QVariantList node_list;
    std::string network_type = neroshop::Script::get_string(neroshop::lua_state, "monero.network_type");
    std::vector<std::string> node_table = neroshop::Script::get_table_string(neroshop::lua_state, coin.toStdString() + ".nodes." + network_type); // Get monero nodes from settings.lua////std::cout << "lua_query: " << coin.toStdString() + ".nodes." + network_type << std::endl;
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
    /*int monerod = Process::get_process_by_name("monerod");
    if(monerod == -1) { return false; }
    std::cout << "\033[1;90;49m" << "monerod is running (ID:" << monerod << ")\033[0m" << std::endl; 
    return true;*/
    
    #ifdef Q_OS_WIN
    QString program = "monerod.exe";
    #else
    QString program = "monerod";
    #endif
    
    QProcess process;
    process.start("pgrep", QStringList() << program); // specific to Linux-based systems
    process.waitForFinished();

    if(process.exitCode() == 0) std::cout << "\033[1;90;49m" << program.toStdString() << " was already running in the background\033[0m" << std::endl;
    return process.exitCode() == 0;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
// TODO: replace function return type with enum
QVariantList neroshop::Backend::validateDisplayName(const QString& display_name) const {
    // username (will appear only in lower-case letters within the app)
    std::string username = display_name.toStdString();
    // Empty display names are acceptable
    if(display_name.isEmpty()) return { true, "" };

    if(!neroshop::string_tools::is_valid_username(username)) {
        std::string default_message = "Invalid username: " + username;
        neroshop::print(default_message, 1);
        if (username.length() < 2) {
            std::string message = "must be at least 2 characters in length";
            return { false, QString::fromStdString(message) };
        }
        if (username.length() > 30) {
            std::string message = "cannot exceed 30 characters in length";
            return { false, QString::fromStdString(message) };
        }
        if (std::regex_search(username, std::regex("\\s"))) {
            std::string message = "cannot contain spaces\n";
            return { false, QString::fromStdString(message) };
        }
        if (!std::regex_search(username, std::regex("^[a-zA-Z]"))) {
            std::string message = "must begin with a letter (cannot start with a symbol or number)";
            return { false, QString::fromStdString(message) };
        }
        if (!std::regex_search(username, std::regex("[a-zA-Z0-9]$"))) {
            std::string message = "must end with a letter or number (cannot end with a symbol)";
            return { false, QString::fromStdString(message) };
        }
        if (std::regex_search(username, std::regex("[^a-zA-Z0-9._-]"))) {
            std::string message = "contains invalid symbol(s) (only '.', '_', and '-' are allowed in between the display name)";
            return { false, QString::fromStdString(message) };
        }
        if (username == "Guest") {
            std::string message = "name \"Guest\" is reserved for guests only and cannot be used by any other user";
            return { false, QString::fromStdString(message) };
        }
        return { false, QString::fromStdString(default_message) };
    }
    
    // Check database to see if display name is available - might not be necessary as user ids are unique
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
    db::Sqlite3 * database = neroshop::get_database();
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
    // TODO: Make sure daemon is connected first
    if(!DaemonManager::isDaemonServerBound()) {
        return { false, "Please wait for the daemon IPC server to connect first" };
    }
    //---------------------------------------------
    db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    //---------------------------------------------
    // Validate display name
    auto name_validation_result = validateDisplayName(display_name);
    if(!name_validation_result[0].toBool()) {
        bool boolean_result = name_validation_result[0].toBool();
        QString message_result = name_validation_result[1].toString();
        return { boolean_result, message_result };
    }
    //---------------------------------------------
    // Get wallet primary address and check its validity
    std::string primary_address = wallet_controller->getPrimaryAddress().toStdString();//neroshop::print("Primary address: \033[1;33m" + primary_address + "\033[1;37m\n");
    if(!wallet_controller->getWallet()->is_valid_address(primary_address)) {
        return { false, "Invalid monero address" };
    }
    //---------------------------------------------
    // Generate RSA key pair (this is for sending/receiving encrypted messages)
    std::string config_path = NEROSHOP_DEFAULT_CONFIGURATION_PATH;
    std::string public_key_filename = config_path + "/" + primary_address + ".pub";
    std::string private_key_filename = config_path + "/" + primary_address + ".key";
    EVP_PKEY * pkey = neroshop::crypto::rsa_generate_keys_get();
    if(pkey == nullptr) {
        return { false, "Failed to generate RSA key pair" };
    }
    // Get a copy of the public key
    std::string public_key = neroshop::crypto::rsa_get_public_key(pkey);
    std::string private_key = neroshop::crypto::rsa_get_private_key(pkey);
    // Save the key pair to disk
    if(!neroshop::crypto::rsa_save_keys(pkey, public_key_filename, private_key_filename)) {
        return { false, "Failed to save RSA key pair" };
    }
    //---------------------------------------------
    // Store login credentials in database
    // Todo: make this command (DB entry) a client request that the server must respond to and the consensus must agree with
    // Note: Multiple users cannot have the same display_name. Each display_name must be unique!
    std::string user_id = database->get_text_params("INSERT INTO users(name, monero_address, public_key) VALUES($1, $2, $3) RETURNING monero_address;", { display_name.toStdString(), primary_address, public_key });//int user_id = database->get_integer_params("INSERT INTO users(name, monero_address) VALUES($1, $2) RETURNING id;", { display_name.toStdString(), primary_address.toStdString() });
    if(user_id.empty()) { return { false, "Account registration failed (due to database error)" }; }//if(user_id == 0) { return { false, "Account registration failed (due to database error)" }; }
    // Create cart for user
    QString cart_uuid = QUuid::createUuid().toString();
    cart_uuid = cart_uuid.remove("{").remove("}"); // remove brackets
    database->execute_params("INSERT INTO cart (uuid, user_id) VALUES ($1, $2)", { cart_uuid.toStdString(), user_id });
    //---------------------------------------------
    // initialize user obj
    std::unique_ptr<neroshop::User> seller(neroshop::Seller::on_login(*wallet_controller->getWallet()));
    user_controller->_user = std::move(seller);
    if (user_controller->getUser() == nullptr) {
        return {false, "user is NULL"};
    }
    user_controller->_user->set_name(display_name.toStdString());
    user_controller->_user->set_public_key(public_key);
    user_controller->_user->set_private_key(private_key);
    //---------------------------------------------
    // Store login credentials in DHT
    Client * client = Client::get_main_client();
    // If client is not connect, return error
    if (!client->is_connected()) return { false, "Not connected to daemon server" };
    // Serialize user object
    auto data = Serializer::serialize(*user_controller->_user);
    std::string key = data.first;
    std::string value = data.second;
    
    // Send put and receive response
    std::string put_response;
    client->put(key, value, put_response);
    std::cout << "Received response: " << put_response << "\n";

    //---------------------------------------------
    emit user_controller->userChanged();
    emit user_controller->userLogged();
    // temp - remove soon
    //user_controller->rateItem("2b715653-da61-4ea0-8b5a-ad2754d78ba1", 3, "This product is aiight");
    user_controller->rateSeller("5AncSFWauoN8bfA68uYpWJRM8fFxEqztuhXSGkeQn5Xd9yU6XqJPW7cZmtYETUAjTK1fCfYQX1CP3Dnmy5a8eUSM5n3C6aL", 1, "This seller rocks");
    // Display registration message
    neroshop::print(((!display_name.isEmpty()) ? "Welcome to neroshop, " : "Welcome to neroshop") + display_name.toStdString(), 4);
    return { true, "" };
}
//----------------------------------------------------------------
bool neroshop::Backend::loginWithWalletFile(WalletController* wallet_controller, const QString& path, const QString& password, UserController * user_controller) { 
    db::Sqlite3 * database = neroshop::get_database();
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
    //----------------------------------------
    // First, check the DHT for user account info
    ////bool dht_account_found = database->get_integer_params("SELECT EXISTS(SELECT DISTINCT key FROM mappings WHERE search_term MATCH ? AND content MATCH 'listing')", { primary_address });
    //----------------------------------------
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
    //----------------------------------------
    // Load RSA keys from file
    std::string config_path = NEROSHOP_DEFAULT_CONFIGURATION_PATH;
    std::string public_key_path = config_path + "/" + primary_address + ".pub";
    std::string private_key_path = config_path + "/" + primary_address + ".key";
    //----------------------------------------
    // Load public_key
    std::ifstream public_key_file(public_key_path);
    if (!public_key_file) {
        // Handle file open error
        throw std::runtime_error("Failed to open public key file: " + public_key_path);
    }

    std::ostringstream buffer0;
    buffer0 << public_key_file.rdbuf();
    std::string public_key = buffer0.str();
    //----------------------------------------
    // Load private_key
    std::ifstream private_key_file(private_key_path);
    if (!private_key_file) {
        // Handle file open error
        throw std::runtime_error("Failed to open private key file: " + private_key_path);
    }

    std::ostringstream buffer;
    buffer << private_key_file.rdbuf();
    std::string private_key = buffer.str();    
    //----------------------------------------
    // Set RSA private key
    user_controller->_user->set_public_key(public_key);
    user_controller->_user->set_private_key(private_key);
    //----------------------------------------
    emit user_controller->userChanged();
    emit user_controller->userLogged();
    // temp - remove soon
    //getSellerRatings(wallet_controller->getPrimaryAddress());
    getSellerRatingsCount("5AncSFWauoN8bfA68uYpWJRM8fFxEqztuhXSGkeQn5Xd9yU6XqJPW7cZmtYETUAjTK1fCfYQX1CP3Dnmy5a8eUSM5n3C6aL");
    getSellerReputation("5AncSFWauoN8bfA68uYpWJRM8fFxEqztuhXSGkeQn5Xd9yU6XqJPW7cZmtYETUAjTK1fCfYQX1CP3Dnmy5a8eUSM5n3C6aL");
    //getProductRatings("2b715653-da61-4ea0-8b5a-ad2754d78ba1");
    /*getProductStarCount("2b715653-da61-4ea0-8b5a-ad2754d78ba1");
    getProductStarCount("2b715653-da61-4ea0-8b5a-ad2754d78ba1", 5);
    getProductStarCount("2b715653-da61-4ea0-8b5a-ad2754d78ba1", 4);
    getProductStarCount("2b715653-da61-4ea0-8b5a-ad2754d78ba1", 3);
    getProductStarCount("2b715653-da61-4ea0-8b5a-ad2754d78ba1", 2);
    getProductStarCount("2b715653-da61-4ea0-8b5a-ad2754d78ba1", 1);
    getProductAverageStars("2b715653-da61-4ea0-8b5a-ad2754d78ba1");*/
    // Display message
    neroshop::print("Welcome back, user " + ((!display_name.empty()) ? (display_name + " (id: " + primary_address + ")") : primary_address), 4);
    return true;
}
//----------------------------------------------------------------
bool neroshop::Backend::loginWithMnemonic(WalletController* wallet_controller, const QString& mnemonic, UserController * user_controller) {
    db::Sqlite3 * database = neroshop::get_database();
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
    db::Sqlite3 * database = neroshop::get_database();
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
    ////Validator::generate_sha256_hash(primary_address, user_auth_key); // temp
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

