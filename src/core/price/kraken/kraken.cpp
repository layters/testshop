#include "kraken.hpp"

#if defined(NEROSHOP_USE_QT)
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#else
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#endif

#include <map>

#include "../currency_map.hpp"
#include "../../../core/tools/string.hpp" // neroshop::string::upper

std::optional<double> KrakenApi::price(neroshop::Currency from, neroshop::Currency to) const
{
    std::string base_url { "https://api.kraken.com/0/public/Ticker?pair=%1%2" }; // "https://api.kraken.com/0/public/Ticker?pair=XMRUSD" or https://api.kraken.com/0/public/OHLC?pair=XMRUSD
    
    // Fill map with initial currency ids
    std::map<neroshop::Currency, std::string> CURRENCY_TO_ID;
    for (const auto& [key, value] : neroshop::CurrencyMap) {
        CURRENCY_TO_ID[std::get<0>(value)] = neroshop::string::upper(key);
    }
    // Fill map with initial currency vs and codes
    std::map<neroshop::Currency, std::string> CURRENCY_TO_VS;
    for (const auto& [key, value] : neroshop::CurrencyMap) {
        CURRENCY_TO_VS[std::get<0>(value)] = neroshop::string::upper(key);
    }    

    auto it = CURRENCY_TO_ID.find(from);
    if (it == CURRENCY_TO_ID.cend()) {
        return std::nullopt;
    }
    const auto id_from = it->second;

    it = CURRENCY_TO_VS.find(to);
    if (it == CURRENCY_TO_VS.cend()) {
        return std::nullopt;
    }
    const auto id_to = it->second;

    #if defined(NEROSHOP_USE_QT)
    QNetworkAccessManager manager;
    QEventLoop loop;
    QObject::connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    const QUrl url(QString::fromStdString(base_url).arg(QString::fromStdString(id_from), QString::fromStdString(id_to)));
    auto reply = manager.get(QNetworkRequest(url));
    loop.exec();
    QJsonParseError error;
    const auto json_doc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return std::nullopt;
    }
    const auto root_obj = json_doc.object();
    
    QJsonValue result_value = root_obj.value("result");
    assert(result_value.isObject()); // "result" must be an object
    QJsonObject result_obj = result_value.toObject();
    
    std::string x_mark {"X"};
    std::string z_mark {"Z"};
    std::string obj_key = (x_mark + id_from) + (z_mark + id_to);//std::cout << obj_key << std::endl; // TEMP
    assert(result_obj.value(QString::fromStdString(obj_key)).isObject()); // "XXMRZUSD" or whatever must be an object
    QJsonObject final_obj = result_obj.value(QString::fromStdString(obj_key)).toObject();

    foreach(const QString& key, final_obj.keys()) {//std::cout << "key: " << key.toStdString() << "\n";
        if(final_obj.value(key).isArray()) {//assert(final_obj.value(key).isArray()); // "a" or whatever must be an array
            QJsonArray price_array = final_obj.value(key).toArray();//std::cout << price_array.at(0).toString().toStdString() << std::endl;
            if(key == "a") {
                if(price_array.at(0).isString())
                    return price_array.at(0).toString().toDouble(); // "a"[0] <- index 0 of array "a"
            }
        }
    }    
    #else
    #endif
    
    return std::nullopt;
}
