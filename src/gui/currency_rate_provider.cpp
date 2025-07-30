#include "currency_rate_provider.hpp"

#include <QFuture>
#include <QMutexLocker>
#include <QQmlEngine>
#include <QTimer>
#include <QtConcurrent>

#include "../core/price_api/currency_map.hpp"
#include "../core/price_api/currency_converter.hpp"

namespace {
const std::vector<neroshop::PriceSource> SOURCES_TO_USE{
    neroshop::PriceSource::CoinMarketCap,
    neroshop::PriceSource::CoinGecko,
    neroshop::PriceSource::CryptoWatch,
    neroshop::PriceSource::CoinTelegraph,
    neroshop::PriceSource::CryptoRank,
    neroshop::PriceSource::CoinCodex,
    neroshop::PriceSource::Fawazahmed0,
    // Exchanges
    neroshop::PriceSource::Kraken,
};

QHash<QPair<int, int>, double> updateRates(const QList<QPair<int, int>> keys)
{
    QHash<QPair<int, int>, double> newRates;
    for (const auto &key : keys) {
        const auto currencyFrom = static_cast<neroshop::Currency>(key.first);
        const auto currencyTo = static_cast<neroshop::Currency>(key.second);

        for (const auto &source : SOURCES_TO_USE) {
            auto price_source = neroshop::Converter::make_price_source(source);
            auto price_opt = price_source->price(currencyFrom, currencyTo);
            if (price_opt.has_value()) {
                newRates[key] = price_opt.value();
                break;
            }
        }
    }
    return newRates;
}
} // namespace

CurrencyExchangeRatesProvider::CurrencyExchangeRatesProvider()
{
    // Initialize mCurrentRates with desired currency pairs:
    QMutexLocker locker(&mRatesMutex);
    mCurrentRates.clear();
    // Example: fill with some desired pairs, like (XMR, USD), (XMR, EUR), etc:

    mCurrentRates.insert(qMakePair(int(neroshop::Currency::XMR), int(neroshop::Currency::USD)), 0.0);
    mCurrentRates.insert(qMakePair(int(neroshop::Currency::XMR), int(neroshop::Currency::EUR)), 0.0);
    // Add other pairs as needed...

    locker.unlock();
    //-----------------------------
    connect(&mUpdateFutureWatcher,
            &QFutureWatcher<QHash<QPair<int, int>, double>>::finished,
            this,
            [this]() {
                // Update rates safely
                this->setRates(mUpdateFutureWatcher.result());
                
                // Schedule next update after delay, non-blocking, in main thread event loop
                QTimer::singleShot(3000, this, &CurrencyExchangeRatesProvider::startUpdate);
            });
            
    startUpdate(); // start first update asynchronously
}

CurrencyExchangeRatesProvider::~CurrencyExchangeRatesProvider()
{
    mUpdateFutureWatcher.waitForFinished();
}

QObject *CurrencyExchangeRatesProvider::qmlInstance(QQmlEngine * /*engine*/,
                                                    QJSEngine * /*scriptEngine*/)
{
    return CurrencyExchangeRatesProvider::instance();
}

CurrencyExchangeRatesProvider* CurrencyExchangeRatesProvider::instance() {
    static CurrencyExchangeRatesProvider instance;
    return &instance;
}

int CurrencyExchangeRatesProvider::reevaluate() const
{
    // this dummy property is used for dynamic recalculation. Don't delete
    return 42;
}

double CurrencyExchangeRatesProvider::getPrice(const QString &from, const QString &to) const
{
    QMutexLocker locker(&mRatesMutex);
    
    const auto fromIt = neroshop::CurrencyMap.find(from.toUpper().toStdString());
    if (fromIt == neroshop::CurrencyMap.cend()) {
        return 0.0;
    }
    const auto currencyFrom = static_cast<int>(std::get<0>(fromIt->second));

    const auto toIt = neroshop::CurrencyMap.find(to.toUpper().toStdString());
    if (toIt == neroshop::CurrencyMap.cend()) {
        return 0.0;
    }
    const auto currencyTo = static_cast<int>(std::get<0>(toIt->second));

    const auto key = qMakePair(currencyFrom, currencyTo);
    if (!mCurrentRates.contains(key)) {
        mCurrentRates.insert(key, 0.0); // returning 0.0 would make price permanently zero
    }

    return mCurrentRates.value(key);
}

void CurrencyExchangeRatesProvider::setRates(const QHash<QPair<int, int>, double> &newRates)
{
    {
        QMutexLocker locker(&mRatesMutex);
        mCurrentRates = newRates;
    } // unlock BEFORE signal emitted
    emit ratesUpdated();
}

void CurrencyExchangeRatesProvider::startUpdate()
{
    if (mUpdateFutureWatcher.isRunning())
        return; // avoid overlapping updates
        
    // Very important: copy keys under lock, but pass copies to QtConcurrent
    QList<QPair<int, int>> keys;
    {
        QMutexLocker locker(&mRatesMutex);
        keys = mCurrentRates.keys();
    }
    
    mUpdateFutureWatcher.setFuture(QtConcurrent::run(updateRates, keys));
}
