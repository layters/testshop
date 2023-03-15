#ifndef CURRENCY_EXCHANGE_RATES_PROVIDER_H
#define CURRENCY_EXCHANGE_RATES_PROVIDER_H

#include <QFutureWatcher>
#include <QHash>
#include <QMutex>
#include <QObject>
#include <QPair>
#include <QTimer>

class QQmlEngine;
class QJSEngine;
class CurrencyExchangeRatesProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int reevaluate READ reevaluate NOTIFY ratesUpdated)
public:
    explicit CurrencyExchangeRatesProvider();
    ~CurrencyExchangeRatesProvider();
    static QObject *qmlInstance(QQmlEngine *engine, QJSEngine *scriptEngine);

    int reevaluate() const;

    Q_INVOKABLE double getPrice(const QString &from, const QString &to) const;

    void setRates(const QHash<QPair<int, int>, double> &newRates);

signals:
    void ratesUpdated();

private:
    void startUpdate();

private:
    mutable QMutex mRatesMutex;
    mutable QHash<QPair<int, int>, double> mCurrentRates;
    QFutureWatcher<QHash<QPair<int, int>, double>> mUpdateFutureWatcher;
};

#endif // CURRENCY_EXCHANGE_RATES_PROVIDER_H
