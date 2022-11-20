#include "coincodex.hpp"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <map>
#include <QString>

namespace {

const QString BASE_URL{QStringLiteral("https://coincodex.com/api/coincodex/get_coin/%1")};

const std::map<neroshop::Currency, QString> CURRENCY_TO_ID{
    {neroshop::Currency::BTC, "btc"},
    {neroshop::Currency::ETH, "eth"},
    {neroshop::Currency::XMR, "xmr"},
};

} // namespace

std::optional<double> CoinCodexPriceSource::price(neroshop::Currency from, neroshop::Currency to) const
{
    // Fill map with initial currency ids and codes
    std::map<neroshop::Currency, QString> CURRENCY_TO_VS;
    for (const auto& [key, value] : neroshop::CurrencyMap) {
        CURRENCY_TO_VS[std::get<0>(value)] = QString::fromStdString(key).toLower();
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

    QNetworkAccessManager manager;
    QEventLoop loop;
    QObject::connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    const QUrl url(BASE_URL.arg(idFrom));
    auto reply = manager.get(QNetworkRequest(url));
    loop.exec();
    QJsonParseError error;
    const auto json_doc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return std::nullopt;
    }
    const auto root_obj = json_doc.object();
    return root_obj.value("last_price_usd").toDouble();
}
