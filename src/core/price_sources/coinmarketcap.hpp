#ifndef COINMARKETCAP_PRICE_SOURCE_HPP_NEROSHOP
#define COINMARKETCAP_PRICE_SOURCE_HPP_NEROSHOP

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

#include "price_source.hpp"

class CoinMarketCapPriceSource : public PriceSource
{
public:
    std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const override;
};

#endif // COINMARKETCAP_PRICE_SOURCE_HPP_NEROSHOP
