#include "coinmarketcap.hpp"

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

const QString BASE_URL{QStringLiteral("https://api.coinmarketcap.com/data-api/v3/"
                                      "cryptocurrency/quote/latest?id=%1&convertId=%2")};

const std::map<neroshop::Currency, QString> CURRENCY_TO_ID{
    {neroshop::Currency::USD, "2781"},
    {neroshop::Currency::AUD, "2782"},
    {neroshop::Currency::CAD, "2784"},
    {neroshop::Currency::CHF, "2785"},
    {neroshop::Currency::CNY, "2787"},
    {neroshop::Currency::EUR, "2790"},
    {neroshop::Currency::GBP, "2791"},
    {neroshop::Currency::JPY, "2797"},
    {neroshop::Currency::MXN, "2799"},
    {neroshop::Currency::NZD, "2802"},
    {neroshop::Currency::SEK, "2807"},
    {neroshop::Currency::NGN, "2819"},
    {neroshop::Currency::GHS, "3540"},
    {neroshop::Currency::RUB, "2806"},
    {neroshop::Currency::PHP, "2803"},
    {neroshop::Currency::INR, "2796"},
    //{neroshop::Currency::},
    //{neroshop::Currency::XAG, ""},
    //{neroshop::Currency::XAU, ""},
    {neroshop::Currency::BTC, "1"},
    {neroshop::Currency::ETH, "1027"},
    {neroshop::Currency::LTC, "2"},    
    {neroshop::Currency::WOW, "4978"},
    {neroshop::Currency::XMR, "328"},
};

}

std::optional<double> CoinMarketCapApi::price(neroshop::Currency from, neroshop::Currency to) const
{
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
    const auto data_val = root_obj.value("data");
    if (!data_val.isArray()) {
        return std::nullopt;
    }
    const auto data_arr = data_val.toArray();
    if (data_arr.empty()) {
        return std::nullopt;
    }
    const auto item = data_arr.first().toObject();
    const auto quote = item.value("quotes").toArray().first().toObject();
    if (!quote.contains("price")) {
        return std::nullopt;
    }
    return quote.value("price").toDouble();
}
