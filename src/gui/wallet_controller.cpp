#include "wallet_controller.hpp"

neroshop::WalletController::WalletController() {
    wallet = std::make_unique<neroshop::Wallet>();
}

neroshop::WalletController::~WalletController() {
    if(wallet.get()) {
        wallet.reset();
    }
    #ifdef NEROSHOP_DEBUG
    std::cout << "wallet controller deleted\n";
    #endif
}

#if defined(NEROSHOP_USE_QT)
////neroshop::WalletController::Wallet(QObject* parent) : QObject(parent) {}

neroshop::Wallet * neroshop::WalletController::getWallet() const {
    return wallet.get();
}

int neroshop::WalletController::createRandomWallet(const QString& password, const QString& confirm_pwd, const QString& path) const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    auto error = wallet->create_random(password.toStdString(), confirm_pwd.toStdString(), path.toStdString());
    return static_cast<int>(error);
}

void neroshop::WalletController::restoreFromMnemonic(const QString& mnemonic) {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    wallet->restore_from_mnemonic(mnemonic.toStdString());
}

//void restoreFromKeys(const QString& primary_address, const QString& private_view_key, const QString& private_spend_key()) {}

bool neroshop::WalletController::open(const QString& path, const QString& password) {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    return wallet->open(path.toStdString(), password.toStdString());
}

void neroshop::WalletController::close(bool save) {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    wallet->get_monero_wallet()->close(save);
    wallet->monero_wallet_obj.reset(); // set monero_wallet to nullptr so that we know it has been deleted
}

QVariantMap neroshop::WalletController::createUniqueSubaddressObject(unsigned int account_idx, const QString & label) {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
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


QString neroshop::WalletController::signMessage(const QString& message) const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    return QString::fromStdString(wallet->sign_message(message.toStdString(), monero_message_signature_type::SIGN_WITH_SPEND_KEY));
}

bool neroshop::WalletController::verifyMessage(const QString& message, const QString& signature) const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    return wallet->verify_message(message.toStdString(), signature.toStdString());
}


int neroshop::WalletController::getNetworkType() const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    return static_cast<int>(wallet->get_network_type());
}

QString neroshop::WalletController::getNetworkTypeString() const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    return QString::fromStdString(wallet->get_network_type_string());
}

QString neroshop::WalletController::getMnemonic() const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) return "";
    return QString::fromStdString(wallet->get_monero_wallet()->get_mnemonic());
}

QStringList neroshop::WalletController::getMnemonicList() const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList seed_phrase = QString::fromStdString(wallet->get_monero_wallet()->get_mnemonic()).split(' ');
    return seed_phrase;
}

QString neroshop::WalletController::getPrimaryAddress() const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    return QString::fromStdString(wallet->get_monero_wallet()->get_primary_address());
}

QStringList neroshop::WalletController::getAddressesAll() const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList addresses;
    for(auto address : wallet->get_addresses_all(0)) {
        addresses << QString::fromStdString(address.m_address.get());//std::cout << address << std::endl;
    }
    return addresses;
}

QStringList neroshop::WalletController::getAddressesUsed() const{
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList addresses;
    for(auto address : wallet->get_addresses_used(0)) {
        addresses << QString::fromStdString(address.m_address.get());//std::cout << address << " (used)" << std::endl;
    }
    return addresses;    
}

QStringList neroshop::WalletController::getAddressesUnused() const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList addresses;
    for(auto address : wallet->get_addresses_unused(0)) {
        addresses << QString::fromStdString(address.m_address.get());//std::cout << address << std::endl;
    }
    return addresses;    
}

double neroshop::WalletController::getBalanceLocked(unsigned int account_index) const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    double piconero = 0.000000000001;
    // primary address balance
    return wallet->get_monero_wallet()->get_balance(account_index) * piconero;
}

double neroshop::WalletController::getBalanceLocked(unsigned int account_index, unsigned int subaddress_index) const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    double piconero = 0.000000000001;
    // subaddress balance
    return wallet->get_monero_wallet()->get_balance(account_index, subaddress_index) * piconero;
}

double neroshop::WalletController::getBalanceUnlocked(unsigned int account_index) const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    double piconero = 0.000000000001;
    // primary address balance unlocked
    return wallet->get_monero_wallet()->get_unlocked_balance(account_index) * piconero;
}

double neroshop::WalletController::getBalanceUnlocked(unsigned int account_index, unsigned int subaddress_index) const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    double piconero = 0.000000000001;
    // subaddress balance unlocked
    return wallet->get_monero_wallet()->get_unlocked_balance(account_index, subaddress_index) * piconero;
}


void neroshop::WalletController::nodeConnect(const QString& ip, const QString& port, const QString& username, const QString& password) {
    wallet->daemon_connect_remote(ip.toStdString(), port.toStdString(), username.toStdString(), password.toStdString());
}

void neroshop::WalletController::daemonConnect(const QString& username, const QString& password) {
    wallet->daemon_connect_local(username.toStdString(), password.toStdString());
}

void neroshop::WalletController::daemonExecute(const QString& daemon_dir, bool confirm_external_bind, bool restricted_rpc, QString data_dir, unsigned int restore_height) {//const {
    wallet->daemon_open(daemon_dir.toStdString(), confirm_external_bind, restricted_rpc, data_dir.toStdString(), restore_height);
}


double neroshop::WalletController::getSyncPercentage() const {
    return wallet->get_sync_percentage();
}

unsigned int neroshop::WalletController::getSyncHeight() const {
    return wallet->get_sync_height();
}

unsigned int neroshop::WalletController::getSyncStartHeight() const {
    return wallet->get_sync_start_height();
}

unsigned int neroshop::WalletController::getSyncEndHeight() const {
    return wallet->get_sync_end_height();
}

QString neroshop::WalletController::getSyncMessage() const {
    return QString::fromStdString(wallet->get_sync_message());
}


void neroshop::WalletController::setNetworkTypeByString(const QString& network_type) {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    wallet->set_network_type_by_string(network_type.toLower().toStdString());
}


bool neroshop::WalletController::isConnectedToDaemon() const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    return wallet->get_monero_wallet()->is_connected_to_daemon();
}

bool neroshop::WalletController::isSynced() const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    return wallet->get_monero_wallet()->is_synced();
}

bool neroshop::WalletController::isDaemonSynced() const {
    if(!wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    if(!wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    if(!wallet->get_monero_wallet()->is_connected_to_daemon()) {
        return false;
    }
    return wallet->get_monero_wallet()->is_daemon_synced(); // will cause crash if wallet is not connected to daemon
}

bool neroshop::WalletController::isOpened() const {
    return (wallet->get_monero_wallet() != nullptr);
}

bool neroshop::WalletController::fileExists(const QString& filename) const {
    return wallet->file_exists(filename.toStdString());
}

#endif
