#include "backend.hpp"

QString neroshop::Backend::urlToLocalFile(const QUrl& url) const {
    return url.toLocalFile();
}

void neroshop::Backend::copyTextToClipboard(const QString& text) {
    QClipboard * clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
    std::cout << "Copied text to clipboard\n";
}

QStringList neroshop::Backend::getCurrencyList() const {
    QStringList currency_list;
    for (const auto& [key, value] : neroshop::CurrencyMap) {
        currency_list << QString::fromStdString(key);
    }
    return currency_list;
}

int neroshop::Backend::getCurrencyDecimals(const QString& currency) const {
    auto map_key = currency.toUpper().toStdString();
    // Check if key exists in std::map
    if(neroshop::CurrencyMap.count(map_key) > 0) {
        auto map_value = neroshop::CurrencyMap[map_key];
        int decimal_places = std::get<2>(map_value);
        return decimal_places;
    }
    return 2;
}

double neroshop::Backend::getPrice(double amount, const QString& currency) const {
    auto map_key = currency.toUpper().toStdString();
    // Check if key exists in std::map
    if(neroshop::CurrencyMap.count(map_key) > 0) {////if(neroshop::CurrencyMap.find(map_key) != neroshop::CurrencyMap.end()) {
        auto map_value = neroshop::CurrencyMap[map_key];
        neroshop::Currency preferred_currency = std::get<0>(map_value);
        return neroshop::Converter::get_price(neroshop::Currency::XMR, preferred_currency) * amount;
    }
    neroshop::print(currency.toUpper().toStdString() + " is not supported", 1);
    return 0.0;
}

QString neroshop::Backend::getCurrencySign(const QString& currency) const {
    return QString::fromStdString(neroshop::Converter::get_currency_sign(currency.toStdString()));
}

bool neroshop::Backend::isSupportedCurrency(const QString& currency) const {
    return neroshop::Converter::is_supported_currency(currency.toStdString());
}


void neroshop::Backend::registerUser() {

}

void neroshop::Backend::loginWithWalletFile() {

}

void neroshop::Backend::loginWithMnemonic() {

}

void neroshop::Backend::loginWithKeys() {

}

void neroshop::Backend::loginWithHW() {

}


void neroshop::Backend::testFunction(gui::Wallet * wallet) {
    std::cout << "Is monero wallet generated: " << (wallet->get_monero_wallet() != nullptr) << std::endl;
}
