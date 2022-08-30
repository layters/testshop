#include "wallet_proxy.hpp"

neroshop::gui::Wallet::Wallet() {
    wallet = std::make_unique<neroshop::Wallet>();
}

neroshop::gui::Wallet::~Wallet() {
    if(wallet.get()) {
        wallet.reset();
    }
    #ifdef NEROSHOP_DEBUG
    std::cout << "wallet proxy deleted\n";
    #endif
}

#if defined(NEROSHOP_USE_QT)
////neroshop::gui::Wallet::Wallet(QObject* parent) : QObject(parent) {}

neroshop::Wallet * neroshop::gui::Wallet::get_wallet() const {
    return wallet.get();
}

void neroshop::gui::Wallet::create_random_wallet(const QString& password, const QString& confirm_pwd, const QString& path) {//const {
    // if the wallet already exists then this will crash so we must check if wallet exists first
    wallet->create_random(password.toStdString(), confirm_pwd.toStdString(), path.toStdString());
}

QString neroshop::gui::Wallet::get_mnemonic() const {
    if(!wallet->get_monero_wallet()) return "";
    return QString::fromStdString(wallet->get_monero_wallet()->get_mnemonic());
}
#endif
