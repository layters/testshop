#include "fawazahmed0.hpp"

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

std::optional<double> Fawazahmed0CurrencyApi::price(neroshop::Currency from, neroshop::Currency to) const
{
    // Fill map with initial currency ids
    std::map<neroshop::Currency, std::string> CURRENCY_TO_ID;
    for (const auto& [key, value] : neroshop::CurrencyMap) {
        CURRENCY_TO_ID[std::get<0>(value)] = neroshop::string::lower(key);
    }
    // Fill map with initial currency vs and codes
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
    const auto idTo = it->second;

    #if defined(NEROSHOP_USE_QT)
    const QString BASE_URL{QStringLiteral("https://cdn.jsdelivr.net/gh/fawazahmed0/currency-api@1/latest/currencies/%1/%2.json")};
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
    return root_obj.value(QString::fromStdString(idTo)).toDouble(); // for date: root_obj.value("date").toString()
    #else
    #endif
    
    return std::nullopt;
}
