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
    if(!wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    auto error = wallet->create_random(password.toStdString(), confirm_pwd.toStdString(), path.toStdString());
    return static_cast<int>(error);
}

void neroshop::gui::Wallet::copyMnemonicToClipboard() {
    if(!wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) return;
    QClipboard * clipboard = QGuiApplication::clipboard();
    clipboard->setText(getMnemonic());
    std::cout << "Copied to clipboard\n";
}

QVariantMap neroshop::gui::Wallet::createUniqueSubaddressObject(unsigned int account_idx, const QString & label) {
    if(!wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    QVariantMap subaddress_object;
    monero::monero_subaddress subaddress = wallet->create_subaddress(account_idx, label.toStdString());
    double piconero = 0.000000000001;
    subaddress_object.insert("account_index", subaddress.m_account_index.get());
    subaddress_object.insert("index", subaddress.m_index.get());
    subaddress_object.insert("address", QString::fromStdString(subaddress.m_address.get()));
    subaddress_object.insert("label", QString::fromStdString(subaddress.m_label.get()));
    subaddress_object.insert("balance", (qulonglong(subaddress.m_balance.get()) * piconero));
    subaddress_object.insert("unlocked_balance", (qulonglong(subaddress.m_unlocked_balance.get()) * piconero));
    subaddress_object.insert("num_unspent_outputs", qulonglong(subaddress.m_num_unspent_outputs.get()));
    subaddress_object.insert("is_used", subaddress.m_is_used.get());
    subaddress_object.insert("num_blocks_to_unlock", qulonglong(subaddress.m_num_blocks_to_unlock.get())); // uint64_t is an unsigned long long so we have to convert it into a qulonglong
    // Usage: console.log("subaddress: ", (!Wallet.isGenerated()) ? "" : Wallet.createUniqueSubaddressObject(0).address)
    return subaddress_object;
}

QString neroshop::gui::Wallet::getMnemonic() const {
    if(!wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) return "";
    return QString::fromStdString(wallet->get_monero_wallet()->get_mnemonic());
}

QStringList neroshop::gui::Wallet::getMnemonicList() const {
    if(!wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList seed_phrase = QString::fromStdString(wallet->get_monero_wallet()->get_mnemonic()).split(' ');
    return seed_phrase;
}

QString neroshop::gui::Wallet::getPrimaryAddress() const {
    if(!wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    return QString::fromStdString(wallet->get_monero_wallet()->get_primary_address());
}

QStringList neroshop::gui::Wallet::getAddressesAll() const {
    if(!wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList addresses;
    for(auto address : wallet->get_addresses_all(0)) {
        addresses << QString::fromStdString(address.m_address.get());//std::cout << address << std::endl;
    }
    return addresses;
}

QStringList neroshop::gui::Wallet::getAddressesUsed() const{
    if(!wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList addresses;
    for(auto address : wallet->get_addresses_used(0)) {
        addresses << QString::fromStdString(address.m_address.get());//std::cout << address << " (used)" << std::endl;
    }
    return addresses;    
}

QStringList neroshop::gui::Wallet::getAddressesUnused() const {
    if(!wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList addresses;
    for(auto address : wallet->get_addresses_unused(0)) {
        addresses << QString::fromStdString(address.m_address.get());//std::cout << address << std::endl;
    }
    return addresses;    
}

double neroshop::gui::Wallet::getBalanceLocked(unsigned int account_index) const {
    if(!wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    double piconero = 0.000000000001;
    // primary address balance
    return wallet->get_monero_wallet()->get_balance(account_index) * piconero;
}

double neroshop::gui::Wallet::getBalanceLocked(unsigned int account_index, unsigned int subaddress_index) const {
    if(!wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    double piconero = 0.000000000001;
    // subaddress balance
    return wallet->get_monero_wallet()->get_balance(account_index, subaddress_index) * piconero;
}

double neroshop::gui::Wallet::getBalanceUnlocked(unsigned int account_index) const {
    if(!wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    double piconero = 0.000000000001;
    // primary address balance unlocked
    return wallet->get_monero_wallet()->get_unlocked_balance(account_index) * piconero;
}

double neroshop::gui::Wallet::getBalanceUnlocked(unsigned int account_index, unsigned int subaddress_index) const {
    if(!wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    double piconero = 0.000000000001;
    // subaddress balance unlocked
    return wallet->get_monero_wallet()->get_unlocked_balance(account_index, subaddress_index) * piconero;
}


void neroshop::gui::Wallet::daemonExecute(const QString& ip, const QString& port, bool confirm_external_bind, bool restricted_rpc, bool remote, QString data_dir, QString network_type, unsigned int restore_height) {//const {
    wallet->daemon_open(ip.toStdString(), port.toStdString(), confirm_external_bind, restricted_rpc, remote, data_dir.toStdString(), network_type.toStdString(), restore_height);
}

void neroshop::gui::Wallet::daemonConnect(const QString& ip, const QString& port) {
    std::cout << "Main thread ID: " << std::this_thread::get_id() << "\n";
    wallet->daemon_connect_remote(ip.toStdString(), port.toStdString());
}

double neroshop::gui::Wallet::getSyncPercentage() const {
    return wallet->get_sync_percentage();
}

unsigned int neroshop::gui::Wallet::getSyncHeight() const {
    return wallet->get_sync_height();
}

unsigned int neroshop::gui::Wallet::getSyncStartHeight() const {
    return wallet->get_sync_start_height();
}

unsigned int neroshop::gui::Wallet::getSyncEndHeight() const {
    return wallet->get_sync_end_height();
}

QString neroshop::gui::Wallet::getSyncMessage() const {
    return QString::fromStdString(wallet->get_sync_message());
}


bool neroshop::gui::Wallet::isGenerated() const {
    return (wallet->get_monero_wallet() != nullptr);
}
#endif
