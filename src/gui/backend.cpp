#include "backend.hpp"

QString neroshop::Backend::urlToLocalFile(const QUrl& url) const {
    return url.toLocalFile();
}

void neroshop::Backend::copyTextToClipboard(const QString& text) {
    QClipboard * clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
    std::cout << "Copied text to clipboard\n";
}


double neroshop::Backend::convertXmr(double amount, const QString& currency, bool to) const {
    return neroshop::Converter::convert_xmr(amount, currency.toStdString(), to);
}

QString neroshop::Backend::getCurrencySymbol(const QString& currency) const {
    return QString::fromStdString(neroshop::Converter::get_currency_symbol(currency.toStdString()));
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
