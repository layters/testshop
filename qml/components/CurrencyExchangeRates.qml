pragma Singleton

import QtQuick 2.12
import neroshop 1.0

QtObject {
    function getPrice(from, to) {
        let reevaluate = CurrencyExchangeRatesProvider.reevaluate;
        return CurrencyExchangeRatesProvider.getPrice(from, to);
    }

    function getXmrPrice(currency) {
        return getPrice("XMR", currency);
    }

    function convertToXmr(amount, currency) {
        let rate = getXmrPrice(currency);
        if (rate > 0.0) {
            return amount / rate;
        }
        return 0.0;
    }
}
