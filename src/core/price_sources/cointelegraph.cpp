#include "cointelegraph.hpp"

namespace {

const QString BASE_URL{QStringLiteral("https://ticker-api.cointelegraph.com/rates/?full=true")};

const std::map<neroshop::Currency, QString> CURRENCY_TO_ID{
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
};

const std::map<neroshop::Currency, QString> CRYPTO_TO_ID{
    {neroshop::Currency::BTC, "BTC"},
    {neroshop::Currency::ETH, "ETH"},
    {neroshop::Currency::XMR, "XMR"},
};

bool is_crypto(neroshop::Currency c)
{
    return CRYPTO_TO_ID.find(c) != CRYPTO_TO_ID.cend();
}

bool is_currency(neroshop::Currency c)
{
    return CURRENCY_TO_ID.find(c) != CURRENCY_TO_ID.cend();
}

} // namespace

std::optional<double> CoinTelegraphPriceSource::price(neroshop::Currency from, neroshop::Currency to) const
{
    if (is_currency(from) && is_currency(to)) {
        return std::nullopt;
    }

    QNetworkAccessManager manager;
    QEventLoop loop;
    QObject::connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    const QUrl url(BASE_URL);
    auto reply = manager.get(QNetworkRequest(url));
    loop.exec();
    QJsonParseError error;
    const auto json_doc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return std::nullopt;
    }
    const auto root_obj = json_doc.object();
    const auto data_val = root_obj.value("data");
    if (!data_val.isObject()) {
        return std::nullopt;
    }
    const auto data_obj = data_val.toObject();

    if (is_crypto(from)) {
        const auto crypto_obj = data_obj.value(CRYPTO_TO_ID.at(from)).toObject();

        if (is_currency(to)) {
            const auto currecy_obj = crypto_obj.value(CURRENCY_TO_ID.at(to)).toObject();
            if (!currecy_obj.contains("price")) {
                return std::nullopt;
            }
            return currecy_obj.value("price").toDouble();
        }

        if (is_crypto(to)) {
            const auto usd_obj = crypto_obj.value("USD").toObject();
            if (!usd_obj.contains("price")) {
                return std::nullopt;
            }
            const auto from_price = usd_obj.value("price").toDouble();

            const auto crypto_obj = data_obj.value(CRYPTO_TO_ID.at(to)).toObject();
            const auto currecy_obj = crypto_obj.value("USD").toObject();
            if (!currecy_obj.contains("price")) {
                return std::nullopt;
            }
            const auto to_price = currecy_obj.value("price").toDouble();
            return from_price / to_price;
        }

        return std::nullopt;
    }

    if (is_currency(from)) {
        const auto crypto_obj = data_obj.value(CRYPTO_TO_ID.at(to)).toObject();
        const auto currecy_obj = crypto_obj.value(CURRENCY_TO_ID.at(from)).toObject();
        if (!currecy_obj.contains("price")) {
            return std::nullopt;
        }
        return 1.0 / currecy_obj.value("price").toDouble();
    }

    return std::nullopt;
}
