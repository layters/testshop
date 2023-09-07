#include "user_controller.hpp"

#include <QDateTime>

#include "../core/cart.hpp"
#include "../core/category.hpp"
#include "../core/database/database.hpp"
#include "../core/protocol/transport/client.hpp"
#include "../core/tools/logger.hpp"
#include "enum_wrapper.hpp"

neroshop::UserController::UserController(QObject *parent) : QObject(parent)
{
    // Should user not be initialized until they are logged in or?
    ////user = std::make_unique<neroshop::Seller>();
}

neroshop::UserController::~UserController() {
    std::cout << "user controller deleted\n";
}
//----------------------------------------------------------------
QString neroshop::UserController::listProduct(const QString& name, const QString& description,
        const QList<QVariantMap>& attributes, 
        const QString& product_code, int category_id, const QList<int>& subcategory_ids, const QStringList& tags, const QList<QVariantMap>& images,
int quantity, double price, const QString& currency, const QString& condition, const QString& location) {
    if (!_user)
        throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());
    
    std::vector<Attribute> attributesVector;
    for (const QVariantMap& attributeMap : attributes) {
        Attribute attribute;

        // Extract values from QVariantMap and assign them to the corresponding members of the Attribute struct
        if (attributeMap.contains("color")) attribute.color = attributeMap.value("color").toString().toStdString();
        if (attributeMap.contains("size")) attribute.size = attributeMap.value("size").toString().toStdString();
        if (attributeMap.contains("weight")) attribute.weight = attributeMap.value("weight").toDouble();
        if (attributeMap.contains("material")) attribute.material = attributeMap.value("material").toString().toStdString();

        /*// Extract and parse the dimensions value
        QVariant dimensionsVariant = attributeMap.value("dimensions");
        if (dimensionsVariant.canConvert<QString>()) {
            QString dimensionsString = dimensionsVariant.toString();
            // Parse the dimensions string and assign the values to the dimensions member
            // You need to implement the parsing logic based on the possible string formats
            attribute.dimensions = parseDimensions(dimensionsString.toStdString());
        }

        attribute.brand = attributeMap.value("brand").toString().toStdString();
        attribute.model = attributeMap.value("model").toString().toStdString();
        attribute.manufacturer = attributeMap.value("manufacturer").toString().toStdString();
        attribute.country_of_origin = attributeMap.value("country_of_origin").toString().toStdString();
        attribute.warranty_information = attributeMap.value("warranty_information").toString().toStdString();
        attribute.product_code = attributeMap.value("product_code").toString().toStdString();
        attribute.style = attributeMap.value("style").toString().toStdString();
        attribute.gender = attributeMap.value("gender").toString().toStdString();

        // Extract and parse the age_range value
        QVariant ageRangeVariant = attributeMap.value("age_range");
        if (ageRangeVariant.canConvert<QVariantList>()) {
            QVariantList ageRangeList = ageRangeVariant.toList();
            if (ageRangeList.size() >= 2) {
                attribute.age_range.first = ageRangeList.at(0).toInt();
                attribute.age_range.second = ageRangeList.at(1).toInt();
            }
        }

        attribute.energy_efficiency_rating = attributeMap.value("energy_efficiency_rating").toString().toStdString();
        attribute.safety_features = attributeMap.value("safety_features").toStringList().toVector();//.toStdVector();
        attribute.quantity_per_package = attributeMap.value("quantity_per_package").toUInt();
        attribute.release_date = attributeMap.value("release_date").toString().toStdString();*/

        // Add the attribute to the vector
        attributesVector.push_back(attribute);
    }
    
    std::vector<int> subcategoryIdVector;
    for(const int& subcategory_id : subcategory_ids) {
        subcategoryIdVector.push_back(subcategory_id);
    }
    
    std::vector<std::string> tagsVector;
    for (const QString& tag : tags) {
        tagsVector.push_back(tag.toStdString());
    }
    
    std::vector<Image> imagesVector;
    for (const QVariantMap& imageMap : images) {
        Image image;
        
        if(imageMap.contains("name")) image.name = imageMap.value("name").toString().toStdString();
        if(imageMap.contains("size")) image.size = imageMap.value("size").toInt();
        if(imageMap.contains("id")) image.id = imageMap.value("id").toInt();
        if(imageMap.contains("source")) image.source = imageMap.value("source").toString().toStdString();
        
        imagesVector.push_back(image);
    }
    
    auto listing_key = seller->list_item(
        name.toStdString(), 
        description.toStdString(),
        attributesVector, 
        product_code.toStdString(),
        category_id, 
        subcategoryIdVector,
        tagsVector,
        imagesVector,
        
        quantity, 
        price, 
        currency.toStdString(), 
        condition.toStdString(), 
        location.toStdString()
    );
    emit productsCountChanged();
    emit inventoryChanged();
    
    return QString::fromStdString(listing_key);
}
//----------------------------------------------------------------
void neroshop::UserController::delistProduct(const QString& listing_key) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());
    seller->delist_item(listing_key.toStdString());
    emit productsCountChanged();
    emit inventoryChanged();
}
//----------------------------------------------------------------
void neroshop::UserController::delistProducts(const QStringList& listing_keys) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());
    for(const auto& listing_key : listing_keys) {
        seller->delist_item(listing_key.toStdString());//std::cout << listing_key.toStdString() << " has been delisted\n";// <- may be "undefined"
    }
    emit productsCountChanged(); // Only emit the signal once we're done delisting all the products
    emit inventoryChanged();
}
//----------------------------------------------------------------
//----------------------------------------------------------------
int neroshop::UserController::addToCart(const QString& listing_key, int quantity) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    int error = _user->add_to_cart(listing_key.toStdString(), quantity);
    emit cartQuantityChanged(); // TODO: emit this when removing an item from the cart as well
    return error;
}
//----------------------------------------------------------------
//void removeFromCart(const QString& listing_key, int quantity) {}
//----------------------------------------------------------------
//----------------------------------------------------------------
Q_INVOKABLE void neroshop::UserController::createOrder(const QString& shipping_address) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    _user->create_order(shipping_address.toStdString());
    emit cartQuantityChanged();
}
//----------------------------------------------------------------
//----------------------------------------------------------------
void neroshop::UserController::rateItem(const QString& product_id, int stars, const QString& comments)
{
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());

    std::string signature = seller->get_wallet()->sign_message(comments.toStdString(), monero_message_signature_type::SIGN_WITH_SPEND_KEY);
    std::cout << "Signature: \033[33m" << signature << "\033[0m" << std::endl;
    std::string verified = (seller->get_wallet()->verify_message(comments.toStdString(), signature)) ? "true" : "false";
    std::cout << "Is verified: " << ((verified == "true") ? "\033[32m" : "\033[31m") << verified << "\033[0m" << std::endl;
    
    _user->rate_item(product_id.toStdString(), stars, comments.toStdString(), signature);//signature.toStdString());
}
//----------------------------------------------------------------
void neroshop::UserController::rateSeller(const QString& seller_id, int score, const QString& comments)
{
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());

    std::string signature = seller->get_wallet()->sign_message(comments.toStdString(), monero_message_signature_type::SIGN_WITH_SPEND_KEY);
    std::cout << "Signature: \033[33m" << signature << "\033[0m" << std::endl;
    std::string verified = (seller->get_wallet()->verify_message(comments.toStdString(), signature)) ? "true" : "false";
    std::cout << "Is verified: " << ((verified == "true") ? "\033[32m" : "\033[31m") << verified << "\033[0m" << std::endl;
    
    _user->rate_seller(seller_id.toStdString(), score, comments.toStdString(), signature);//signature.toStdString());
}
//----------------------------------------------------------------
//----------------------------------------------------------------
void neroshop::UserController::addToFavorites(const QString& listing_key) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    _user->add_to_favorites(listing_key.toStdString());
}
//----------------------------------------------------------------
void neroshop::UserController::removeFromFavorites(const QString& listing_key) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    _user->remove_from_favorites(listing_key.toStdString());
}
//----------------------------------------------------------------
bool neroshop::UserController::hasFavorited(const QString& listing_key) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    return _user->has_favorited(listing_key.toStdString());
}
//----------------------------------------------------------------
//----------------------------------------------------------------
void neroshop::UserController::uploadAvatar(const QString& filename) {
    if (!_user)
        throw std::runtime_error("neroshop::User is not initialized");
    _user->upload_avatar(filename.toStdString());
}
//----------------------------------------------------------------
void neroshop::UserController::sendMessage(const QString& recipient_id, const QString& content, 
    const QString& public_key) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    _user->send_message(recipient_id.toStdString(), content.toStdString(), public_key.toStdString());
}
//----------------------------------------------------------------
QVariantMap neroshop::UserController::decryptMessage(const QString& content_encoded, const QString& sender_encoded) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    std::pair<std::string, std::string> message_decrypted = _user->decrypt_message(content_encoded.toStdString(), sender_encoded.toStdString());
    
    QVariantMap message;
    message["content"] = QString::fromStdString(message_decrypted.first);
    message["sender_id"] = QString::fromStdString(message_decrypted.second);
    return message;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
void neroshop::UserController::setStockQuantity(const QString& listing_key, int quantity) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());
    seller->set_stock_quantity(listing_key.toStdString(), quantity);
    emit productsCountChanged();
    emit inventoryChanged();
}
//----------------------------------------------------------------
//----------------------------------------------------------------    
neroshop::User * neroshop::UserController::getUser() const {
    return _user.get();
}
//----------------------------------------------------------------
neroshop::Seller * neroshop::UserController::getSeller() const {
    return static_cast<neroshop::Seller *>(_user.get());
}
//----------------------------------------------------------------
QString neroshop::UserController::getId() const {
    if (!_user)
        throw std::runtime_error("neroshop::User is not initialized");
    return QString::fromStdString(_user->get_id());
}
//----------------------------------------------------------------
int neroshop::UserController::getProductsCount() const {
    if (!_user)
        throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());
    return seller->get_products_count();
}
//----------------------------------------------------------------
int neroshop::UserController::getReputation() const {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());
    return seller->get_reputation();
}
//----------------------------------------------------------------
//----------------------------------------------------------------
int neroshop::UserController::getCartQuantity() const {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    return _user->get_cart()->get_quantity();//(_user->get_id());
}
//----------------------------------------------------------------
//----------------------------------------------------------------
// This code causes the images to not load for some reason unless the app is restarted. Edit: fixed
QVariantList neroshop::UserController::getInventory(int sorting) const {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    
    Client * client = Client::get_main_client();
    
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    std::string command = "SELECT DISTINCT key FROM mappings WHERE search_term = ?1 AND content = 'listing'";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    // Bind user_id to TEXT
    std::string user_id = _user->get_id();
    if(sqlite3_bind_text(stmt, 1, user_id.c_str(), user_id.length(), SQLITE_STATIC) != SQLITE_OK) {
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
        QVariantList product_images;
        QStringList product_categories;

        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));//std::cout << column_value  << " (" << i << ")" << std::endl;
            if(column_value == "NULL") continue; // Skip invalid columns
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
                //emit productsCountChanged();
                //emit inventoryChanged();
                continue; // Key is lost or missing from DHT, skip to next iteration
            }
            
            const auto& response_obj = json["response"];
            assert(response_obj.is_object());
            if (response_obj.contains("value") && response_obj["value"].is_string()) {
                const auto& value = response_obj["value"].get<std::string>();
                nlohmann::json value_obj = nlohmann::json::parse(value);
                assert(value_obj.is_object());//std::cout << value_obj.dump(4) << "\n";
                std::string metadata = value_obj["metadata"].get<std::string>();
                if (metadata != "listing") { std::cerr << "Invalid metadata. \"listing\" expected, got \"" << metadata << "\" instead\n"; continue; }
                inventory_object.insert("key", key);
                inventory_object.insert("listing_uuid", QString::fromStdString(value_obj["id"].get<std::string>()));
                inventory_object.insert("seller_id", QString::fromStdString(value_obj["seller_id"].get<std::string>()));
                inventory_object.insert("quantity", value_obj["quantity"].get<int>());
                inventory_object.insert("price", value_obj["price"].get<double>());
                inventory_object.insert("currency", QString::fromStdString(value_obj["currency"].get<std::string>()));
                inventory_object.insert("condition", QString::fromStdString(value_obj["condition"].get<std::string>()));
                if(value_obj.contains("location") && value_obj["location"].is_string()) {
                    inventory_object.insert("location", QString::fromStdString(value_obj["location"].get<std::string>()));
                }
                inventory_object.insert("date", QString::fromStdString(value_obj["date"].get<std::string>()));
                assert(value_obj["product"].is_object());
                const auto& product_obj = value_obj["product"];
                inventory_object.insert("product_uuid", QString::fromStdString(product_obj["id"].get<std::string>()));
                inventory_object.insert("product_name", QString::fromStdString(product_obj["name"].get<std::string>()));
                inventory_object.insert("product_description", QString::fromStdString(product_obj["description"].get<std::string>()));
                // product category and subcategories
                std::string category = product_obj["category"].get<std::string>();
                product_categories.append(QString::fromStdString(category));
                if (product_obj.contains("subcategories") && product_obj["subcategories"].is_array()) {
                    const auto& subcategories_array = product_obj["subcategories"];
                    for (const auto& subcategory : subcategories_array) {
                        if (subcategory.is_string()) {
                            product_categories.append(QString::fromStdString(subcategory.get<std::string>()));
                        }
                    }
                    inventory_object.insert("product_categories", product_categories);
                }
                //inventory_object.insert("", QString::fromStdString(product_obj[""].get<std::string>()));
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
                    inventory_object.insert("product_images", product_images);
                }
                if (product_obj.contains("thumbnail") && product_obj["thumbnail"].is_string()) {
                    inventory_object.insert("product_thumbnail", QString::fromStdString(product_obj["thumbnail"].get<std::string>()));
                }
            }
            inventory_array.append(inventory_object);
        }
    }
        
    sqlite3_finalize(stmt);
        
    switch(sorting) {
        case static_cast<int>(EnumWrapper::Sorting::SortNone):
            std::cout << "SortNone (" << static_cast<int>(EnumWrapper::Sorting::SortNone) << ") selected\n";
            break;
        case static_cast<int>(EnumWrapper::Sorting::SortByAvailability): // Filter items with quantity less than 1
            std::cout << "SortByAvailability (" << static_cast<int>(EnumWrapper::Sorting::SortByAvailability) << ") selected\n";
            inventory_array.erase(std::remove_if(inventory_array.begin(), inventory_array.end(), [](const QVariant& variant) {
                const QVariantMap& inventory_object = variant.toMap();
                return inventory_object.value("quantity").toInt() < 1;
            }), inventory_array.end());
            break;
        case static_cast<int>(EnumWrapper::Sorting::SortByQuantitySmallest):
            std::sort(inventory_array.begin(), inventory_array.end(), [](const QVariant& a, const QVariant& b) {
                const QVariantMap& itemA = a.toMap();
                const QVariantMap& itemB = b.toMap();
                return itemA.value("quantity").toInt() < itemB.value("quantity").toInt();
            });
            break;            
        case static_cast<int>(EnumWrapper::Sorting::SortByQuantityBiggest):
            std::sort(inventory_array.begin(), inventory_array.end(), [](const QVariant& a, const QVariant& b) {
                const QVariantMap& itemA = a.toMap();
                const QVariantMap& itemB = b.toMap();
                return itemA.value("quantity").toInt() > itemB.value("quantity").toInt();
            });     
            break;
        case static_cast<int>(EnumWrapper::Sorting::SortByDateOldest):
            std::sort(inventory_array.begin(), inventory_array.end(), [](const QVariant& a, const QVariant& b) {
                QVariantMap listingA = a.toMap();
                QVariantMap listingB = b.toMap();
                QString dateA = listingA["date"].toString();
                QString dateB = listingB["date"].toString();
                
                // Convert 'Z' to UTC+0 offset
                if (dateA.endsWith("Z")) {
                    dateA.replace(dateA.length() - 1, 1, "+00:00");
                }
                if (dateB.endsWith("Z")) {
                    dateB.replace(dateB.length() - 1, 1, "+00:00");
                }
                
                QDateTime dateTimeA = QDateTime::fromString(dateA, Qt::ISODateWithMs);
                QDateTime dateTimeB = QDateTime::fromString(dateB, Qt::ISODateWithMs);

                return dateTimeA < dateTimeB;
            });
            break;
        case static_cast<int>(EnumWrapper::Sorting::SortByDateNewest):
            std::sort(inventory_array.begin(), inventory_array.end(), [](const QVariant& a, const QVariant& b) {
                QVariantMap listingA = a.toMap();
                QVariantMap listingB = b.toMap();
                QString dateA = listingA["date"].toString();
                QString dateB = listingB["date"].toString();
                
                // Convert 'Z' to UTC+0 offset
                if (dateA.endsWith("Z")) {
                    dateA.replace(dateA.length() - 1, 1, "+00:00");
                }
                if (dateB.endsWith("Z")) {
                    dateB.replace(dateB.length() - 1, 1, "+00:00");
                }
                
                QDateTime dateTimeA = QDateTime::fromString(dateA, Qt::ISODateWithMs);
                QDateTime dateTimeB = QDateTime::fromString(dateB, Qt::ISODateWithMs);

                return dateTimeA > dateTimeB;
            });
            break;            
        case static_cast<int>(EnumWrapper::Sorting::SortByPriceLowest):
            std::sort(inventory_array.begin(), inventory_array.end(), [](const QVariant& a, const QVariant& b) {
                QVariantMap listingA = a.toMap();
                QVariantMap listingB = b.toMap();
                return listingA["price"].toDouble() < listingB["price"].toDouble();
            });
            break;
        case static_cast<int>(EnumWrapper::Sorting::SortByPriceHighest):
            std::sort(inventory_array.begin(), inventory_array.end(), [](const QVariant& a, const QVariant& b) {
                QVariantMap listingA = a.toMap();
                QVariantMap listingB = b.toMap();
                return listingA["price"].toDouble() > listingB["price"].toDouble();
            });
            break;             
        /*case static_cast<int>(EnumWrapper::Sorting::SortByName):
            std::cout << "SortByName (" << SortByName << ") selected\n";
            command = "SELECT DISTINCT * FROM listings JOIN products ON products.uuid = listings.product_id JOIN images ON images.product_id = listings.product_id WHERE seller_id = $1 GROUP BY images.product_id;";
            break;*/
        /*case static_cast<int>(EnumWrapper::Sorting::) :
            std::cout << "Sort? (" << ? << ") selected\n";
            break;*/
        default:
            std::cout << "default: SortNone (" << static_cast<int>(EnumWrapper::Sorting::SortNone) << ") selected\n";
            break;
    }

    return inventory_array;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
QVariantList neroshop::UserController::getMessages() const {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    Client * client = Client::get_main_client();
    
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    std::string command = "SELECT DISTINCT key FROM mappings WHERE search_term = ?1 AND content = 'message'";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return {};
    }
    // Bind user_id to TEXT
    std::string user_id = _user->get_id();
    if(sqlite3_bind_text(stmt, 1, user_id.c_str(), user_id.length(), SQLITE_STATIC) != SQLITE_OK) {
        neroshop::print("sqlite3_bind_text (arg: 1): " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(stmt);
        return {};
    }    
    
    QVariantList messages_array;
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        QVariantMap message_object; // Create an object for each row

        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));//std::cout << column_value  << " (" << i << ")" << std::endl;
            if(column_value == "NULL") continue; // Skip invalid columns
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
                continue; // Key is lost or missing from DHT, skip to next iteration
            }
            
            const auto& response_obj = json["response"];
            assert(response_obj.is_object());
            if (response_obj.contains("value") && response_obj["value"].is_string()) {
                const auto& value = response_obj["value"].get<std::string>();
                nlohmann::json value_obj = nlohmann::json::parse(value);
                assert(value_obj.is_object());//std::cout << value_obj.dump(4) << "\n";
                std::string metadata = value_obj["metadata"].get<std::string>();
                if (metadata != "message") { std::cerr << "Invalid metadata. \"message\" expected, got \"" << metadata << "\" instead\n"; continue; }
                message_object.insert("key", key);
                std::string content = value_obj["content"].get<std::string>();
                std::string sender_id = value_obj["sender_id"].get<std::string>();
                auto message_decrypted = _user->decrypt_message(content, sender_id);
                message_object.insert("content", QString::fromStdString(message_decrypted.first));
                message_object.insert("sender_id", QString::fromStdString(message_decrypted.second));
                message_object.insert("recipient_id", QString::fromStdString(value_obj["recipient_id"].get<std::string>()));
                message_object.insert("timestamp", QString::fromStdString(value_obj["timestamp"].get<std::string>()));
            }
            messages_array.append(message_object);
        }
    }
    
    std::sort(messages_array.begin(), messages_array.end(), [](const QVariant& a, const QVariant& b) {
        QVariantMap mapA = a.toMap();
        QVariantMap mapB = b.toMap();
        QString timestampA = mapA.value("timestamp").toString();
        QString timestampB = mapB.value("timestamp").toString();
        
        // Convert 'Z' to UTC+0 offset
        if (timestampA.endsWith("Z")) {
            timestampA.replace(timestampA.length() - 1, 1, "+00:00");
        }
        if (timestampB.endsWith("Z")) {
            timestampB.replace(timestampB.length() - 1, 1, "+00:00");
        }
                
        QDateTime dateTimeA = QDateTime::fromString(timestampA, Qt::ISODateWithMs);
        QDateTime dateTimeB = QDateTime::fromString(timestampB, Qt::ISODateWithMs);
        
        return dateTimeA > dateTimeB;
    });
    
    sqlite3_finalize(stmt);
    
    return messages_array;
}
//----------------------------------------------------------------
QVariantList neroshop::UserController::getMessages(const QString& sender_id) const {
    QVariantList allMessages = getMessages();
    QVariantList filteredMessages;

    // Filter the messages based on the specified sender_id
    for (const QVariant &message : allMessages) {
        QVariantMap messageMap = message.toMap();
        if (messageMap["sender_id"].toString() == sender_id) {
            filteredMessages.append(messageMap);
        }
    }

    return filteredMessages;
}
//----------------------------------------------------------------
QVariantList neroshop::UserController::getMessages(const QString& sender_id, const QVariantList& messages) const {
    QVariantList filteredMessages;

    // Filter the messages based on the specified sender_id
    for (const QVariant &message : messages) {
        QVariantMap messageMap = message.toMap();
        if (messageMap["sender_id"].toString() == sender_id) {
            filteredMessages.append(messageMap);
        }
    }

    return filteredMessages;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
bool neroshop::UserController::isUserLogged() const {
    return (_user.get() != nullptr);
}
//----------------------------------------------------------------
// to allow seller to use user functions: dynamic_cast<Seller *>(user)
