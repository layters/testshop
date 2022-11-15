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
#endif

#include <map>
#if defined(NEROSHOP_USE_QT)
#include <QString>
#endif

namespace {

const QString BASE_URL{QStringLiteral("https://coincodex.com/api/coincodex/get_coin/%1")};

const std::map<neroshop::Currency, QString> CURRENCY_TO_ID{
    {neroshop::Currency::BTC, "btc"},
    {neroshop::Currency::ETH, "eth"},
    {neroshop::Currency::XMR, "xmr"},
};

const std::map<neroshop::Currency, QString> CURRENCY_TO_VS{
    {neroshop::Currency::USD, "usd"},
};

} // namespace

std::optional<double> CoinCodexPriceSource::price(neroshop::Currency from, neroshop::Currency to) const
{
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
