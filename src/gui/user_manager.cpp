#include "user_manager.hpp"

#include <QDateTime>

#include "../core/market/cart.hpp"
#include "../core/market/category.hpp"
#include "../core/database/database.hpp"
#include "../core/protocol/transport/client.hpp"
#include "../core/tools/logger.hpp"
#include "enum_wrapper.hpp"
#include "../core/protocol/rpc/protobuf.hpp"

neroshop::UserManager::UserManager(QObject *parent) : QObject(parent)
{
    // Should user not be initialized until they are logged in or?
    ////user = std::make_unique<neroshop::Seller>();
}

neroshop::UserManager::~UserManager() {
#ifdef NEROSHOP_DEBUG
    std::cout << "user manager deleted\n";
#endif
}
//----------------------------------------------------------------
QString neroshop::UserManager::listProduct(
    const QString& name, 
    const QString& description,
    double weight,
    const QList<QVariantMap>& variants, 
    const QString& productCode, 
    int categoryId, 
    const QList<int>& subcategoryIds, 
    const QStringList& tags, 
    const QList<QVariantMap>& images,

    int quantity, 
    double price, 
    const QString& currency, 
    const QString& condition, 
    const QString& location, 
    int quantityPerOrder, 
    /*int paymentMethod,*/ 
    const QList<int>& paymentCoins, 
    const QList<int>& paymentOptions, 
    const QList<int>& deliveryOptions,
    const QList<int>& shippingOptions,
    const QList<QVariantMap>& shippingCosts,
    const QList<QVariantMap>& customRates
) {
    if (!_user)
        throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());
    
    std::vector<ProductVariant> variantsList;
    for (const QVariantMap& variantMap : variants) {
        ProductVariant variant;
        ProductVariantInfo info;
        
        for (auto it = variantMap.begin(); it != variantMap.end(); ++it) {
            std::string key = it.key().toStdString();
            
            if (key == "options" && it.value().canConvert<QVariantMap>()) {
                QVariantMap optionsMap = it.value().toMap();
                for (auto optIt = optionsMap.begin(); optIt != optionsMap.end(); ++optIt) {
                    variant.options[optIt.key().toStdString()] = optIt.value().toString().toStdString();
                }
            }
            else if (key == "condition") {
                info.condition = it.value().toString().toStdString();
            }
            else if (key == "weight") {
                bool ok = false;
                double w = it.value().toDouble(&ok); // .toInt(&ok) or .toDouble(&ok) attempts the conversion and sets ok to true or false based on success
                if (ok) info.weight = w;
            } 
            else if (key == "price") {
                bool ok = false;
                double p = it.value().toDouble(&ok);
                if (ok) info.price = p;
            }
            else if (key == "quantity") {
                bool ok = false;
                int q = it.value().toInt(&ok);
                if (ok) info.quantity = q;
            }
            else if (key == "product_code") {
                info.product_code = it.value().toString().toStdString();
            }
            else if (key == "images") {
                // TODO:
            }
            else if (key == "image_index") {
                bool ok = false;
                int idx = it.value().toInt(&ok);
                if (ok) info.image_index = idx;
            }
            else {
                // Assume it's an option key with a string value (works with non-nested flat options e.g. {"option": "Color", "value": "Red"})
                QString optionValue = it.value().toString();
                if (!optionValue.isEmpty()) {
                    variant.options[key] = optionValue.toStdString();
                }
            }
        }
        variant.info = std::move(info);
        variantsList.push_back(std::move(variant));
    }
    
    std::set<int> subcategoryIdsSet;
    for(const int& subcategoryId : subcategoryIds) {
        subcategoryIdsSet.insert(subcategoryId);
    }
    
    std::set<std::string> tagsSet;
    for (const QString& tag : tags) {
        tagsSet.insert(tag.toStdString());
    }
    
    std::vector<Image> imagesVector;
    for (const QVariantMap& imageMap : images) {
        Image image;
        std::vector<std::string> pieces;
        std::vector<unsigned char> data;
        
        if(imageMap.contains("name")) image.name = imageMap.value("name").toString().toStdString();
        if(imageMap.contains("size")) image.size = imageMap.value("size").toInt();
        if(imageMap.contains("id")) image.id = imageMap.value("id").toInt();
        if(imageMap.contains("source")) image.source = imageMap.value("source").toString().toStdString();
        if(imageMap.contains("pieces") && imageMap.value("pieces").canConvert<QStringList>()) {
            QStringList piecesList = imageMap.value("pieces").toStringList();
            for(const QString& pieceHashStr : piecesList) {
                pieces.push_back(pieceHashStr.toStdString());
            }
            image.pieces = pieces;
        }
        if(imageMap.contains("piece_size")) image.piece_size = imageMap.value("piece_size").toInt();
        if(imageMap.contains("data") && imageMap.value("data").canConvert<QByteArray>()) {
            QByteArray imageData = imageMap.value("data").toByteArray();
            data.reserve(imageData.size());  // Reserve space to avoid reallocations
            for (int i = 0; i < imageData.size(); ++i) {
                data.push_back(static_cast<unsigned char>(imageData.at(i)));
            }
            image.data = data;
        }
        if(imageMap.contains("width")) image.width = imageMap.value("width").toInt();
        if(imageMap.contains("height")) image.height = imageMap.value("height").toInt();
        
        imagesVector.push_back(image);
    }
    
    std::set<PaymentCoin> paymentCoinsSet;
    for(const int& paymentCoin : paymentCoins) {
        paymentCoinsSet.insert(static_cast<PaymentCoin>(paymentCoin));
    }
    
    std::set<PaymentOption> paymentOptionsSet;
    for(const int& paymentOption : paymentOptions) {
        paymentOptionsSet.insert(static_cast<PaymentOption>(paymentOption));
    }
    
    std::set<DeliveryOption> deliveryOptionsSet;
    for(const int& deliveryOption : deliveryOptions) {
        deliveryOptionsSet.insert(static_cast<DeliveryOption>(deliveryOption));
    }
    
    std::set<ShippingOption> shippingOptionsSet;
    for(const int& shippingOption : shippingOptions) {
        shippingOptionsSet.insert(static_cast<ShippingOption>(shippingOption));
    }
    
    std::map<ShippingOption, double> shippingCostsMap;
    for (const QVariantMap& shippingCost : shippingCosts) {
        if(shippingCost.contains("shipping_option") && shippingCost.contains("cost")) {
            QVariant shipping_option = shippingCost.value("shipping_option");
            QVariant cost = shippingCost.value("cost");
            if(shipping_option.canConvert<int>() && cost.canConvert<double>()) {
                shippingCostsMap.insert({
                    static_cast<ShippingOption>(shipping_option.toInt()),
                    cost.toDouble()
                });
            }
        }
    }
    
    std::map<PaymentCoin, double> customRatesMap;
    for (const QVariantMap& customRate : customRates) {
        if(customRate.contains("payment_coin") && customRate.contains("rate")) {
            QVariant payment_coin = customRate.value("payment_coin");
            QVariant rate = customRate.value("rate");
            if(payment_coin.canConvert<int>() && rate.canConvert<double>()) {
                customRatesMap.insert({
                    static_cast<PaymentCoin>(payment_coin.toInt()),
                    rate.toDouble()
                });
            }
        }
    }
    
    auto listing_key = seller->list_item(
        name.toStdString(), 
        description.toStdString(),
        weight,
        variantsList, 
        productCode.toStdString(),
        categoryId, 
        subcategoryIdsSet,
        tagsSet,
        imagesVector,
        
        quantity, 
        price, 
        currency.toStdString(), 
        condition.toStdString(), 
        location.toStdString(), 
        quantityPerOrder, 
        /*static_cast<PaymentMethod>(paymentMethod),*/ 
        paymentCoinsSet, 
        paymentOptionsSet, 
        deliveryOptionsSet, 
        shippingOptionsSet,
        shippingCostsMap,
        customRatesMap
    );
    emit productsCountChanged();
    emit inventoryChanged();
    
    return QString::fromStdString(listing_key);
}
//----------------------------------------------------------------
void neroshop::UserManager::delistProduct(const QString& listing_key) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());
    seller->delist_item(listing_key.toStdString());
    emit productsCountChanged();
    emit inventoryChanged();
}
//----------------------------------------------------------------
void neroshop::UserManager::delistProducts(const QStringList& listing_keys) {
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
int neroshop::UserManager::addToCart(const QString& listing_key, int quantity) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    int error = _user->add_to_cart(listing_key.toStdString(), quantity);
    emit cartQuantityChanged(); // TODO: emit this when removing an item from the cart as well
    return error;
}
//----------------------------------------------------------------
//void removeFromCart(const QString& listing_key, int quantity) {}
//----------------------------------------------------------------
//----------------------------------------------------------------
Q_INVOKABLE void neroshop::UserManager::createOrder(const QString& shipping_address) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    _user->create_order(shipping_address.toStdString());
    emit cartQuantityChanged();
}
//----------------------------------------------------------------
//----------------------------------------------------------------
void neroshop::UserManager::rateItem(const QString& product_id, int stars, const QString& comments)
{
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());

    std::string signature = seller->get_wallet()->sign_message(comments.toStdString(), monero_message_signature_type::SIGN_WITH_SPEND_KEY);
    std::cout << "Signed: \033[33m" << signature << "\033[0m" << std::endl;
    
    _user->rate_item(product_id.toStdString(), stars, comments.toStdString(), signature);//signature.toStdString());
}
//----------------------------------------------------------------
void neroshop::UserManager::rateSeller(const QString& seller_id, int score, const QString& comments)
{
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());

    std::string signature = seller->get_wallet()->sign_message(comments.toStdString(), monero_message_signature_type::SIGN_WITH_SPEND_KEY);
    std::cout << "Signed: \033[33m" << signature << "\033[0m" << std::endl;
    
    _user->rate_seller(seller_id.toStdString(), score, comments.toStdString(), signature);//signature.toStdString());
}
//----------------------------------------------------------------
//----------------------------------------------------------------
void neroshop::UserManager::addToFavorites(const QString& listing_key) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    _user->add_to_favorites(listing_key.toStdString());
}
//----------------------------------------------------------------
void neroshop::UserManager::removeFromFavorites(const QString& listing_key) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    _user->remove_from_favorites(listing_key.toStdString());
}
//----------------------------------------------------------------
bool neroshop::UserManager::hasFavorited(const QString& listing_key) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    return _user->has_favorited(listing_key.toStdString());
}
//----------------------------------------------------------------
//----------------------------------------------------------------
void neroshop::UserManager::uploadAvatar(const QString& filename) {
    if (!_user)
        throw std::runtime_error("neroshop::User is not initialized");
    _user->upload_avatar(filename.toStdString());
}
//----------------------------------------------------------------
void neroshop::UserManager::sendMessage(const QString& recipient_id, const QString& content, 
    const QString& public_key) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    _user->send_message(recipient_id.toStdString(), content.toStdString(), public_key.toStdString());
}
//----------------------------------------------------------------
QVariantMap neroshop::UserManager::decryptMessage(const QString& content_encoded, const QString& sender_encoded) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    std::pair<std::string, std::string> message_decrypted = _user->decrypt_message(content_encoded.toStdString(), sender_encoded.toStdString());
    
    QVariantMap message;
    message["content"] = QString::fromStdString(message_decrypted.first);
    message["sender_id"] = QString::fromStdString(message_decrypted.second);
    return message;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
void neroshop::UserManager::setStockQuantity(const QString& listing_key, int quantity) {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());
    seller->set_stock_quantity(listing_key.toStdString(), quantity);
    emit productsCountChanged();
    emit inventoryChanged();
}
//----------------------------------------------------------------
//----------------------------------------------------------------    
neroshop::User * neroshop::UserManager::getUser() const {
    return _user.get();
}
//----------------------------------------------------------------
neroshop::Seller * neroshop::UserManager::getSeller() const {
    return static_cast<neroshop::Seller *>(_user.get());
}
//----------------------------------------------------------------
QString neroshop::UserManager::getId() const {
    if (!_user)
        throw std::runtime_error("neroshop::User is not initialized");
    return QString::fromStdString(_user->get_id());
}
//----------------------------------------------------------------
int neroshop::UserManager::getProductsCount() const {
    if (!_user)
        throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());
    return seller->get_products_count();
}
//----------------------------------------------------------------
int neroshop::UserManager::getReputation() const {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    auto seller = dynamic_cast<neroshop::Seller *>(_user.get());
    return seller->get_reputation();
}
//----------------------------------------------------------------
//----------------------------------------------------------------
int neroshop::UserManager::getCartQuantity() const {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    return _user->get_cart()->get_quantity();//(_user->get_id());
}
//----------------------------------------------------------------
//----------------------------------------------------------------
// This code causes the images to not load for some reason unless the app is restarted. Edit: fixed
QVariantList neroshop::UserManager::getInventory(int sorting) const {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    
    Client * client = Client::get_main_client();
    
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    std::string command = "SELECT DISTINCT key FROM mappings WHERE search_term = ?1 AND content = 'listing'";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::log_error("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())));
        return {};
    }
    // Bind user_id to TEXT
    std::string user_id = _user->get_id();
    if(sqlite3_bind_text(stmt, 1, user_id.c_str(), user_id.length(), SQLITE_STATIC) != SQLITE_OK) {
        neroshop::log_error("sqlite3_bind_text (arg: 1): " + std::string(sqlite3_errmsg(database->get_handle())));
        sqlite3_finalize(stmt);
        return {};
    }    
    // Check whether the prepared statement returns no data (for example an UPDATE)
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::log_error("No data found. Be sure to use an appropriate SELECT statement");
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
            std::vector<uint8_t> response;
            client->get(key.toStdString(), response);
            // Skip empty and invalid responses
            if(response.empty()) continue;
            #if defined(NEROSHOP_USE_PROTOBUF)
            neroshop::DhtMessage resp_msg;
            if (!resp_msg.ParseFromArray(response.data(), static_cast<int>(response.size()))) {
                continue; // Parsing error, skip to next iteration
            }
            if (resp_msg.has_error()) {
                log_trace("Received error (get): {}", resp_msg.error().DebugString());
                // Remove obsolete keys from our datastore
                std::vector<uint8_t> response2;
                client->remove(key.toStdString(), response2);
                neroshop::DhtMessage resp_msg2;
                if (resp_msg2.ParseFromArray(response2.data(), static_cast<int>(response2.size()))) {
                    if(resp_msg2.has_response()) log_info("Removed key {}", key.toStdString());//log_trace("Received response (remove): {}", resp_msg2.response().DebugString());
                }
                //emit productsCountChanged();
                //emit inventoryChanged();
                continue; // Key is lost or missing from DHT, skip to next iteration
            } else if(resp_msg.has_response()) {
                log_trace("Received response (get): {}", resp_msg.response().DebugString());
            }
            
            const auto& payload = resp_msg.response().response();
            const auto& data_map = payload.data();
            if (data_map.find("value") != data_map.end()) {
                std::string value = data_map.at("value");
            #endif
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
                if(value_obj.contains("quantity_per_order") && value_obj["quantity_per_order"].is_number_integer()) {
                    inventory_object.insert("quantity_per_order", value_obj["quantity_per_order"].get<int>());
                }
                if(value_obj.contains("payment_coins") && value_obj["payment_coins"].is_array()) {
                    const auto& payment_coins_array = value_obj["payment_coins"];
                    QStringList paymentCoinsList;
                    for (const auto& payment_coin : payment_coins_array) {
                        if(payment_coin.is_string()) {
                            paymentCoinsList << QString::fromStdString(payment_coin);
                        }
                    }
                    inventory_object.insert("payment_coins", paymentCoinsList);
                }
                if(value_obj.contains("payment_options") && value_obj["payment_options"].is_array()) {
                    const auto& payment_options_array = value_obj["payment_options"];
                    QStringList paymentOptionsList;
                    for (const auto& payment_option : payment_options_array) {
                        if(payment_option.is_string()) {
                            paymentOptionsList << QString::fromStdString(payment_option);
                        }
                    }
                    inventory_object.insert("payment_options", paymentOptionsList);
                }
                if(value_obj.contains("delivery_options") && value_obj["delivery_options"].is_array()) {
                    const auto& delivery_options_array = value_obj["delivery_options"];
                    QStringList deliveryOptionsList;
                    for (const auto& delivery_option : delivery_options_array) {
                        if(delivery_option.is_string()) {
                            deliveryOptionsList << QString::fromStdString(delivery_option);
                        }
                    }
                    inventory_object.insert("delivery_options", deliveryOptionsList);
                }                
                if(value_obj.contains("shipping_options") && value_obj["shipping_options"].is_array()) {
                    const auto& shipping_options_array = value_obj["shipping_options"];
                    QStringList shippingOptionsList;
                    for (const auto& shipping_option : shipping_options_array) {
                        if(shipping_option.is_string()) {
                            shippingOptionsList << QString::fromStdString(shipping_option);
                        }
                    }
                    inventory_object.insert("shipping_options", shippingOptionsList);
                }
                if(value_obj.contains("expiration_date") && value_obj["expiration_date"].is_string()) {
                    inventory_object.insert("expiration_date", QString::fromStdString(value_obj["expiration_date"].get<std::string>()));
                }
                assert(value_obj["product"].is_object());
                const auto& product_obj = value_obj["product"];
                ////inventory_object.insert("product_uuid", QString::fromStdString(product_obj["id"].get<std::string>()));
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
                }
                inventory_object.insert("product_categories", product_categories);
                //inventory_object.insert("", QString::fromStdString(product_obj[""].get<std::string>()));
                // product variant_options
                if (product_obj.contains("variants") && product_obj["variants"].is_array()) {
                    QVariantList variantList;
                    const auto& variants_array = product_obj["variants"];
    
                    for (const auto& variant_obj : variants_array) {
                        if (!variant_obj.is_object()) continue;

                        QVariantMap variantMap;

                        // Parse "options" map
                        if (variant_obj.contains("options") && variant_obj["options"].is_object()) {
                            QVariantMap optionsMap;
                            for (auto it = variant_obj["options"].begin(); it != variant_obj["options"].end(); ++it) {
                                if (it.value().is_string()) {
                                    optionsMap.insert(QString::fromStdString(it.key()), QString::fromStdString(it.value().get<std::string>()));
                                } else {
                                    // Just in case, convert other types to string too
                                    optionsMap.insert(QString::fromStdString(it.key()), QString::fromStdString(it.value().dump()));
                                }
                            }
                            variantMap.insert("options", optionsMap);
                        }
        
                        // Parse variant info fields
                        if (variant_obj.contains("weight")) {
                            if (variant_obj["weight"].is_number()) {
                                variantMap.insert("weight", variant_obj["weight"].get<double>());
                            } else if (variant_obj["weight"].is_string()) {
                                bool ok = false;
                                double w = QString::fromStdString(variant_obj["weight"].get<std::string>()).toDouble(&ok);
                                if (ok) variantMap.insert("weight", w);
                            }
                        }
                        
                        if (variant_obj.contains("price")) {
                            if (variant_obj["price"].is_number()) {
                                variantMap.insert("price", variant_obj["price"].get<double>());
                            }
                        }

                        if (variant_obj.contains("quantity")) {
                            if (variant_obj["quantity"].is_number_integer()) {
                                variantMap.insert("quantity", variant_obj["quantity"].get<int>());
                            }
                        }

                        if (variant_obj.contains("product_code") && variant_obj["product_code"].is_string()) {
                            variantMap.insert("product_code", QString::fromStdString(variant_obj["product_code"].get<std::string>()));
                        }

                        if (variant_obj.contains("image_index")) {
                            if (variant_obj["image_index"].is_number_integer()) {
                                variantMap.insert("image_index", variant_obj["image_index"].get<int>());
                            }
                        }

                        if (variant_obj.contains("images") && variant_obj["images"].is_array()) {
                            /*QStringList imagesList;
                            for (const auto& img : variant_obj["images"]) {
                                if (img.is_string()) {
                                    imagesList.append(QString::fromStdString(img.get<std::string>()));
                                }
                            }
                            variantMap.insert("images", imagesList);*/ // <-- will deal with this another time...
                        }

                        variantList.append(variantMap);
                    }
                    inventory_object.insert("product_variants", variantList);
                }
                // product images
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
QVariantList neroshop::UserManager::getMessages() const {
    if (!_user) throw std::runtime_error("neroshop::User is not initialized");
    Client * client = Client::get_main_client();
    
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    std::string command = "SELECT DISTINCT key FROM mappings WHERE search_term = ?1 AND content = 'message'";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::log_error("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())));
        return {};
    }
    // Bind user_id to TEXT
    std::string user_id = _user->get_id();
    if(sqlite3_bind_text(stmt, 1, user_id.c_str(), user_id.length(), SQLITE_STATIC) != SQLITE_OK) {
        neroshop::log_error("sqlite3_bind_text (arg: 1): " + std::string(sqlite3_errmsg(database->get_handle())));
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
            std::vector<uint8_t> response;
            client->get(key.toStdString(), response);
            // Skip empty and invalid responses
            if(response.empty()) continue;
            #if defined(NEROSHOP_USE_PROTOBUF)
            neroshop::DhtMessage resp_msg;
            if (!resp_msg.ParseFromArray(response.data(), static_cast<int>(response.size()))) {
                continue; // Parsing error, skip to next iteration
            }
            if (resp_msg.has_error()) {
                log_trace("Received error (get): {}", resp_msg.error().DebugString());
                // Remove obsolete keys from our datastore
                std::vector<uint8_t> response2;
                client->remove(key.toStdString(), response2);
                neroshop::DhtMessage resp_msg2;
                if (resp_msg2.ParseFromArray(response2.data(), static_cast<int>(response2.size()))) {
                    if(resp_msg2.has_response()) log_info("Removed key {}", key.toStdString());//log_trace("Received response (remove): {}", resp_msg2.response().DebugString());
                }
                continue; // Key is lost or missing from DHT, skip to next iteration
            } else if(resp_msg.has_response()) {
                log_trace("Received response (get): {}", resp_msg.response().DebugString());
            }
            
            const auto& payload = resp_msg.response().response();
            const auto& data_map = payload.data();
            if (data_map.find("value") != data_map.end()) {
                std::string value = data_map.at("value");
            #endif
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
                std::string signature = value_obj["signature"].get<std::string>();
                auto verified = _user->get_wallet()->get_monero_wallet()->verify_message(message_decrypted.first, message_decrypted.second, signature).m_is_good;
                message_object.insert("verified", verified);
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
QVariantList neroshop::UserManager::getMessages(const QString& sender_id) const {
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
QVariantList neroshop::UserManager::getMessages(const QString& sender_id, const QVariantList& messages) const {
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
int neroshop::UserManager::getMessagesCount() const {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    std::string command = "SELECT COUNT(DISTINCT key) FROM mappings WHERE search_term = ?1 AND content = 'message'";
    int messages_count = database->get_integer_params(command, { _user->get_id() });
    //emit messagesCountChanged();
    return messages_count;
}
//----------------------------------------------------------------
//----------------------------------------------------------------
bool neroshop::UserManager::isUserLogged() const {
    return (_user.get() != nullptr);
}
//----------------------------------------------------------------
// to allow seller to use user functions: dynamic_cast<Seller *>(user)
