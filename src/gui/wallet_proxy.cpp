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

neroshop::Wallet * neroshop::gui::Wallet::getWallet() const {
    return wallet.get();
}

int neroshop::gui::Wallet::createRandomWallet(const QString& password, const QString& confirm_pwd, const QString& path) const {
    auto error = wallet->create_random(password.toStdString(), confirm_pwd.toStdString(), path.toStdString());
    return static_cast<int>(error);
}

void neroshop::gui::Wallet::copyMnemonicToClipboard() {
    if(!wallet->get_monero_wallet()) return;
    QClipboard * clipboard = QGuiApplication::clipboard();
    clipboard->setText(getMnemonic());
    std::cout << "Copied to clipboard\n";
}

QString neroshop::gui::Wallet::getMnemonic() const {
    if(!wallet->get_monero_wallet()) return "";
    return QString::fromStdString(wallet->get_monero_wallet()->get_mnemonic());
}

QStringList neroshop::gui::Wallet::getMnemonicModel() const {
    QStringList seed_phrase = QString::fromStdString(wallet->get_monero_wallet()->get_mnemonic()).split(' ');
    return seed_phrase;
}

void neroshop::gui::Wallet::daemonOpen(const QString& ip, const QString& port, bool confirm_external_bind, bool restricted_rpc, bool remote, QString data_dir, QString network_type, unsigned int restore_height) {//const {
    wallet->daemon_open(ip.toStdString(), port.toStdString(), confirm_external_bind, restricted_rpc, remote, data_dir.toStdString(), network_type.toStdString(), restore_height);
}

void neroshop::gui::Wallet::daemonConnect(const QString& ip, const QString& port) {
    bool synced = wallet->daemon_connect(ip.toStdString(), port.toStdString());
}
#endif
