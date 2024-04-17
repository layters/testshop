#include "coincodex.hpp"

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
#include "../../../core/tools/string.hpp" // neroshop::string::lower

std::optional<double> CoinCodexApi::price(neroshop::Currency from, neroshop::Currency to) const
{
    // Fill map with initial currency ids and codes
    const std::map<neroshop::Currency, std::string> CURRENCY_TO_ID{
        {neroshop::Currency::BTC, "btc"},
        {neroshop::Currency::ETH, "eth"},
        {neroshop::Currency::XMR, "xmr"},
    };

    std::map<neroshop::Currency, std::string> CURRENCY_TO_VS;
    for (const auto& [key, value] : neroshop::CurrencyMap) {
        CURRENCY_TO_VS[std::get<0>(value)] = neroshop::string::lower(key);
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
    #if defined(NEROSHOP_USE_QT)
    const QString BASE_URL{QStringLiteral("https://coincodex.com/api/coincodex/get_coin/%1")};
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
    return root_obj.value("last_price_usd").toDouble();
    #else
    #endif
    
    return std::nullopt;
}
