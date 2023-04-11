#include "cryptowatch.hpp"

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

#include "../../core/currency_map.hpp"
#include "../../core/util.hpp" // neroshop::string::lower

std::optional<double> CryptoWatchApi::price(neroshop::Currency from, neroshop::Currency to) const
{
    // Fill map with initial currency ids and codes
    std::map<neroshop::Currency, std::string> CURRENCY_TO_ID;
    for (const auto& [key, value] : neroshop::CurrencyMap) {
        CURRENCY_TO_ID[std::get<0>(value)] = neroshop::string::lower(key);
    }

    auto it = CURRENCY_TO_ID.find(from);
    if (it == CURRENCY_TO_ID.cend()) {
        return std::nullopt;
    }
    const auto idFrom = it->second;

    it = CURRENCY_TO_ID.find(to);
    if (it == CURRENCY_TO_ID.cend()) {
        return std::nullopt;
    }
    const auto idTo = it->second;

    #if defined(NEROSHOP_USE_QT)
    const QString BASE_URL{QStringLiteral("https://billboard.service.cryptowat.ch/"
                                      "markets?sort=price&onlyBaseAssets=%1&onlyQuoteAssets=%2")};
    QNetworkAccessManager manager;
    QEventLoop loop;
    QObject::connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    const QUrl url(BASE_URL.arg(QString::fromStdString(idFrom), QString::fromStdString(idTo)));
    auto reply = manager.get(QNetworkRequest(url));
    loop.exec();
    QJsonParseError error;
    const auto json_doc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return std::nullopt;
    }
    const auto root_obj = json_doc.object();
    const auto data_val = root_obj.value("result");
    if (!data_val.isObject()) {
        return std::nullopt;
    }
    const auto data_obj = data_val.toObject();
    const auto rows_val = data_obj.value("rows");
    if (!rows_val.isArray()) {
        return std::nullopt;
    }
    std::size_t valid_prices = 0;
    double sum_price = 0.0;
    const auto rows_arr = rows_val.toArray();
    for (const auto &row_val : rows_arr) {
        const auto row_obj = row_val.toObject();
        const auto last_price_obj = row_obj.value("lastPriceByAsset").toObject();
        if (last_price_obj.contains(QString::fromStdString(idTo))) {
            sum_price += last_price_obj.value(QString::fromStdString(idTo)).toDouble();
            ++valid_prices;
        }
    }
    if (valid_prices > 0) {
        return sum_price / valid_prices;
    }
    #else
    #endif
    
    return std::nullopt;
}
