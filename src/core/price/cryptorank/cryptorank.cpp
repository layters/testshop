#include "cryptorank.hpp"

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
#include "../../../core/tools/string.hpp" // neroshop::string_tools::upper

std::optional<double> CryptoRankApi::price(neroshop::Currency from, neroshop::Currency to) const
{
    // Fill map with initial currency ids and codes
    const std::map<neroshop::Currency, std::string> CURRENCY_TO_ID{
        {neroshop::Currency::BTC, "bitcoin"},
        {neroshop::Currency::ETH, "ethereum"},
        {neroshop::Currency::XMR, "monero"},
    };

    std::map<neroshop::Currency, std::string> CURRENCY_TO_VS;
    for (const auto& [key, value] : neroshop::CurrencyMap) {
        CURRENCY_TO_VS[std::get<0>(value)] = neroshop::string_tools::upper(key);
    }
    
    auto it = CURRENCY_TO_ID.find(from);
    if (it == CURRENCY_TO_ID.cend()) {
        return std::nullopt;
    }
    const auto idFrom = it->second;

    it = CURRENCY_TO_VS.find(to);
    if (it == CURRENCY_TO_VS.cend()) {
        return std::nullopt;
    }
    const auto idTo = it->second;

    #if defined(NEROSHOP_USE_QT)
    const QString BASE_URL{QStringLiteral("https://api.cryptorank.io/v0/coins/%1?locale=en")};
    QNetworkAccessManager manager;
    QEventLoop loop;
    QObject::connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    const QUrl url(BASE_URL.arg(QString::fromStdString(idFrom)));
    auto reply = manager.get(QNetworkRequest(url));
    loop.exec();
    QJsonParseError error;
    const auto json_doc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return std::nullopt;
    }
    const auto root_obj = json_doc.object();
    const auto data_obj = root_obj.value("data").toObject();
    const auto price_obj = data_obj.value("price").toObject();
    if (!price_obj.contains(QString::fromStdString(idTo))) {
        return std::nullopt;
    }
    return price_obj.value(QString::fromStdString(idTo)).toDouble();
    #else
    #endif
    
    return std::nullopt;
}
