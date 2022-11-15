#include "coingecko.hpp"

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

const QString BASE_URL{
    QStringLiteral("https://api.coingecko.com/api/v3/simple/price?ids=%1&vs_currencies=%2")};

const std::map<neroshop::Currency, QString> CURRENCY_TO_ID{
    {neroshop::Currency::BTC, "bitcoin"},
    {neroshop::Currency::ETH, "ethereum"},
    {neroshop::Currency::XMR, "monero"},
};

const std::map<neroshop::Currency, QString> CURRENCY_TO_VS{
    {neroshop::Currency::USD, "usd"},
    {neroshop::Currency::AUD, "aud"},
    {neroshop::Currency::CAD, "cad"},
    {neroshop::Currency::CHF, "chf"},
    {neroshop::Currency::CNY, "cny"},
    {neroshop::Currency::EUR, "eur"},
    {neroshop::Currency::GBP, "gpb"},
    {neroshop::Currency::JPY, "jpy"},
    {neroshop::Currency::MXN, "mxn"},
    {neroshop::Currency::NZD, "nzd"},
    {neroshop::Currency::SEK, "sek"},
    {neroshop::Currency::BTC, "btc"},
    {neroshop::Currency::ETH, "eth"},
    {neroshop::Currency::XMR, "xmr"},
};

} // namespace

std::optional<double> CoinGeckoPriceSource::price(neroshop::Currency from, neroshop::Currency to) const
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
    const auto idTo = it->second;

    QNetworkAccessManager manager;
    QEventLoop loop;
    QObject::connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    const QUrl url(BASE_URL.arg(idFrom, idTo));
    auto reply = manager.get(QNetworkRequest(url));
    loop.exec();
    QJsonParseError error;
    const auto json_doc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return std::nullopt;
    }
    const auto root_obj = json_doc.object();
    const auto price_obj = root_obj.value(idFrom).toObject();
    return price_obj.value(idTo).toDouble();
}
