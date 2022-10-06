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

int neroshop::gui::Wallet::create_random_wallet(const QString& password, const QString& confirm_pwd, const QString& path) const {
    auto error = wallet->create_random(password.toStdString(), confirm_pwd.toStdString(), path.toStdString());
    return static_cast<int>(error);
}

QString neroshop::gui::Wallet::get_mnemonic() const {
    if(!wallet->get_monero_wallet()) return "";
    return QString::fromStdString(wallet->get_monero_wallet()->get_mnemonic());
}

void neroshop::gui::Wallet::daemonOpen(const QString& ip, const QString& port, bool confirm_external_bind, bool restricted_rpc, bool remote, QString data_dir, QString network_type, unsigned int restore_height) {//const {
    wallet->daemon_open(ip.toStdString(), port.toStdString(), confirm_external_bind, restricted_rpc, remote, data_dir.toStdString(), network_type.toStdString(), restore_height);
}

void neroshop::gui::Wallet::daemonConnect(const QString& ip, const QString& port) {
    bool synced = wallet->daemon_connect(ip.toStdString(), port.toStdString());
}
#endif
