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
#endif

#include <map>
#if defined(NEROSHOP_USE_QT)
#include <QString>
#endif

namespace {

const QString BASE_URL{QStringLiteral("https://api.cryptorank.io/v0/coins/%1?locale=en")};

const std::map<neroshop::Currency, QString> CURRENCY_TO_ID{
    {neroshop::Currency::BTC, "bitcoin"},
    {neroshop::Currency::ETH, "ethereum"},
    {neroshop::Currency::XMR, "monero"},
};

const std::map<neroshop::Currency, QString> CURRENCY_TO_VS{
    {neroshop::Currency::USD, "USD"},
    {neroshop::Currency::AUD, "AUD"},
    {neroshop::Currency::CAD, "CAD"},
    {neroshop::Currency::CHF, "CHF"},
    {neroshop::Currency::CNY, "CNY"},
    {neroshop::Currency::EUR, "EUR"},
    {neroshop::Currency::GBP, "GPB"},
    {neroshop::Currency::JPY, "JPY"},
    {neroshop::Currency::MXN, "MXN"},
    {neroshop::Currency::NZD, "NZD"},
    {neroshop::Currency::SEK, "SEK"},
    {neroshop::Currency::BTC, "BTC"},
    {neroshop::Currency::ETH, "ETH"},
    {neroshop::Currency::XMR, "XMR"},
};

} // namespace

std::optional<double> CryptoRankPriceSource::price(neroshop::Currency from, neroshop::Currency to) const
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

    const QUrl url(BASE_URL.arg(idFrom));
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
    if (!price_obj.contains(idTo)) {
        return std::nullopt;
    }
    return price_obj.value(idTo).toDouble();
}
