#include "image_loader.hpp"
#include <QCoreApplication>
#include <QFile>

#include "../neroshop_config.hpp"
#include "../core/database/database.hpp"
#include "../core/tools/logger.hpp"

ImageLoader::ImageLoader(QObject *parent)
    : QObject{parent}
{
}

ImageLoader *ImageLoader::instance()
{
    static ImageLoader *instance{nullptr};

    if (!instance) {
        instance = new ImageLoader(QCoreApplication::instance());
    }

    return instance;
}
#include <iostream>
#include <QDebug>
#include <QUrlQuery>
QImage ImageLoader::load(const QString &id) const
{
    // TODO: load from real file/base64/DB/...
    // 'id' could be smth like:
    // - "/user/avatar/41234" for avatar image of user with id 41234
    // - "/catalog/item/56542/0" for 0 of N images of a catalog item with id 56542

    // You can use https://doc.qt.io/qt-5/qurlquery.html for parsing id

    QImage result;
    // listings
    if(id.contains(QStringLiteral("listing"))) {
        // Copy id text to a url (image://catalog?id=%1&index=%2)
        QUrl url(QString(id).replace('/', '?'));
        // Construct query object and parse the query string found in the url, using the default query delimiters (=, &)
        QUrlQuery query(url);////query.setQueryDelimiters('=', '&'); // Default delimiters//QString queryString = query.query();std::cout << "queryString: " << queryString.toStdString() << std::endl;
        if(!query.isEmpty()) {
            if(!query.hasQueryItem(QString("id"))) {
                std::cout << "key 'id' not found in query\n";
            }
            
            if(!query.hasQueryItem(QString("image_id"))) {
                std::cout << "key 'image_id' not found in query. Using default image\n";
            }        
            
            if(query.hasQueryItem(QString("image_id"))) {
                // Get key-value pair
                QString listing_key = query.queryItemValue("id");//std::cout << "id: " << listing_key.toStdString() << std::endl;
                QString image_name = query.queryItemValue(QString("image_id"));//std::cout << "image_id: " << image_id.toStdString() << std::endl;

                // Note: Images with large resolutions (ex. 4000 pixels) will crash this code for some reason
        
                QString filename = getProductImagePath(listing_key, image_name);//std::cout << "Loading image from: " << filename.toStdString() << "\n";

                if (!filename.isEmpty()) {
                    QFile file(filename);
                    if (file.open(QFile::ReadOnly)) {
                        result.loadFromData(file.readAll());
                    }
                }
            }
        }
    }
    // avatars
    if(id.contains(QStringLiteral("avatar"))) {
        // Copy id text to a url
        QUrl url(QString(id).replace('/', '?'));
        // Construct query object and parse the query string found in the url, using the default query delimiters (=, &)
        QUrlQuery query(url);
        if(!query.isEmpty()) {
            if(!query.hasQueryItem(QString("id"))) {
                std::cout << "key 'id' not found in query\n";
            }
            
            if(!query.hasQueryItem(QString("image_id"))) {
                std::cout << "key 'image_id' not found in query. Using default image\n";
            }        
            
            if(query.hasQueryItem(QString("image_id"))) {
                // Get key-value pair
                QString user_key = query.queryItemValue("id");//std::cout << "id: " << user_key.toStdString() << std::endl;
                QString image_name = query.queryItemValue(QString("image_id"));//std::cout << "image_id: " << image_id.toStdString() << std::endl;

                // Note: Images with large resolutions (ex. 4000 pixels) will crash this code for some reason
        
                QString filename = getAvatarImagePath(user_key, image_name);//std::cout << "Loading image from: " << filename.toStdString() << "\n";

                if (!filename.isEmpty()) {
                    QFile file(filename);
                    if (file.open(QFile::ReadOnly)) {
                        result.loadFromData(file.readAll());
                    }
                }
            }    
        }
    }

    return result;
}

QString ImageLoader::getProductImagePath(const QString& listing_key, const QString& image_name) const {
    std::string config_path = NEROSHOP_DEFAULT_CONFIGURATION_PATH;
    std::string data_folder = config_path + "/" + NEROSHOP_DATA_FOLDER_NAME;
    std::string listings_folder = data_folder + "/" + NEROSHOP_CATALOG_FOLDER_NAME;
    std::string listing_key_folder = listings_folder + "/" + listing_key.toStdString();
    
    std::string product_image = listing_key_folder + "/" + image_name.toStdString();
    
    return QString::fromStdString(product_image);
}

QString ImageLoader::getAvatarImagePath(const QString& user_key, const QString& image_name) const {
    std::string config_path = NEROSHOP_DEFAULT_CONFIGURATION_PATH;
    std::string data_folder = config_path + "/" + NEROSHOP_DATA_FOLDER_NAME;
    std::string avatars_folder = data_folder + "/" + NEROSHOP_AVATAR_FOLDER_NAME;
    std::string user_key_folder = avatars_folder + "/" + user_key.toStdString();
    
    std::string avatar_image = user_key_folder + "/" + image_name.toStdString();
    
    return QString::fromStdString(avatar_image);
}

static void preload()
{
    ImageLoader::instance();
}

Q_COREAPP_STARTUP_FUNCTION(preload)
