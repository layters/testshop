#include "wallet_controller.hpp"

neroshop::WalletController::WalletController(QObject *parent) : QObject(parent)
{
    _wallet = std::make_unique<neroshop::Wallet>();
}

neroshop::WalletController::~WalletController() {
    #ifdef NEROSHOP_DEBUG
    std::cout << "wallet controller deleted\n";
    #endif
}

#if defined(NEROSHOP_USE_QT)
////neroshop::WalletController::Wallet(QObject* parent) : QObject(parent) {}

neroshop::Wallet * neroshop::WalletController::getWallet() const {
    return _wallet.get();
}

// TODO: replace function return type with enum
int neroshop::WalletController::createRandomWallet(const QString& password, const QString& confirm_pwd, const QString& path) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    auto error = _wallet->create_random(password.toStdString(),
                                        confirm_pwd.toStdString(),
                                        path.toStdString());
    emit walletChanged();
    if(error == neroshop::wallet_error::WALLET_SUCCESS) emit isOpenedChanged();
    return static_cast<int>(error);
}

bool neroshop::WalletController::restoreFromMnemonic(const QString& mnemonic) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    bool restored = _wallet->restore_from_mnemonic(mnemonic.toStdString());
    emit walletChanged();
    if(restored) emit isOpenedChanged();
    return restored;
}

bool neroshop::WalletController::restoreFromKeys(const QString& primary_address, const QString& private_view_key, const QString& private_spend_key) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    bool restored = _wallet->restore_from_keys(primary_address.toStdString(),
                                               private_view_key.toStdString(),
                                               private_spend_key.toStdString());
    emit walletChanged();
    if(restored) emit isOpenedChanged();
    return restored;
}

bool neroshop::WalletController::open(const QString& path, const QString& password) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    bool opened = _wallet->open(path.toStdString(), password.toStdString());
    emit walletChanged();
    if(opened) emit isOpenedChanged();
    return opened;
}

void neroshop::WalletController::close(bool save) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    _wallet->get_monero_wallet()->close(save);
    // set monero_wallet to nullptr so that we know it has been deleted
    _wallet->monero_wallet_obj.reset();
    emit walletChanged();
    emit isOpenedChanged();
}

QVariantMap neroshop::WalletController::createUniqueSubaddressObject(unsigned int account_idx, const QString & label) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    QVariantMap subaddress_object;
    monero::monero_subaddress subaddress = _wallet->create_subaddress(account_idx,
                                                                      label.toStdString());
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


void neroshop::WalletController::transfer(const QString& address, double amount) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    _wallet->transfer(address.toStdString(), amount);
}

QString neroshop::WalletController::signMessage(const QString& message) const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return QString::fromStdString(
        _wallet->sign_message(message.toStdString(),
                              monero_message_signature_type::SIGN_WITH_SPEND_KEY));
}

bool neroshop::WalletController::verifyMessage(const QString& message, const QString& signature) const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->verify_message(message.toStdString(), signature.toStdString());
}


int neroshop::WalletController::getNetworkType() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return static_cast<int>(_wallet->get_network_type());
}

QString neroshop::WalletController::getNetworkTypeString() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return QString::fromStdString(_wallet->get_network_type_string());
}

QString neroshop::WalletController::getMnemonic() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    if (!_wallet->get_monero_wallet())
        return "";
    return QString::fromStdString(_wallet->get_monero_wallet()->get_mnemonic());
}

QStringList neroshop::WalletController::getMnemonicList() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList seed_phrase = QString::fromStdString(_wallet->get_monero_wallet()->get_mnemonic())
                                  .split(' ');
    return seed_phrase;
}

QString neroshop::WalletController::getPrimaryAddress() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    if (!_wallet->get_monero_wallet())
        throw std::runtime_error("monero_wallet_full is not opened");
    return QString::fromStdString(_wallet->get_monero_wallet()->get_primary_address());
}

QStringList neroshop::WalletController::getAddressesAll() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList addresses;
    for (auto address : _wallet->get_addresses_all(0)) {
        addresses << QString::fromStdString(address.m_address.get());//std::cout << address << std::endl;
    }
    return addresses;
}

QStringList neroshop::WalletController::getAddressesUsed() const{
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList addresses;
    for (auto address : _wallet->get_addresses_used(0)) {
        addresses << QString::fromStdString(address.m_address.get());//std::cout << address << " (used)" << std::endl;
    }
    return addresses;    
}

QStringList neroshop::WalletController::getAddressesUnused() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList addresses;
    for (auto address : _wallet->get_addresses_unused(0)) {
        addresses << QString::fromStdString(address.m_address.get());//std::cout << address << std::endl;
    }
    return addresses;    
}

double neroshop::WalletController::getBalanceLocked() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    if (!_wallet->get_monero_wallet())
        throw std::runtime_error("monero_wallet_full is not opened");
    double piconero = 0.000000000001;
    return _wallet->get_monero_wallet()->get_balance() * piconero;
}

double neroshop::WalletController::getBalanceLocked(unsigned int account_index) const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    if (!_wallet->get_monero_wallet())
        throw std::runtime_error("monero_wallet_full is not opened");
    double piconero = 0.000000000001;
    // primary address balance
    return _wallet->get_monero_wallet()->get_balance(account_index) * piconero;
}

double neroshop::WalletController::getBalanceLocked(unsigned int account_index, unsigned int subaddress_index) const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    if (!_wallet->get_monero_wallet())
        throw std::runtime_error("monero_wallet_full is not opened");
    double piconero = 0.000000000001;
    // subaddress balance
    return _wallet->get_monero_wallet()->get_balance(account_index, subaddress_index) * piconero;
}

double neroshop::WalletController::getBalanceUnlocked() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    if (!_wallet->get_monero_wallet())
        throw std::runtime_error("monero_wallet_full is not opened");
    double piconero = 0.000000000001;
    return _wallet->get_monero_wallet()->get_unlocked_balance() * piconero;
}

double neroshop::WalletController::getBalanceUnlocked(unsigned int account_index) const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    if (!_wallet->get_monero_wallet())
        throw std::runtime_error("monero_wallet_full is not opened");
    double piconero = 0.000000000001;
    // primary address balance unlocked
    return _wallet->get_monero_wallet()->get_unlocked_balance(account_index) * piconero;
}

double neroshop::WalletController::getBalanceUnlocked(unsigned int account_index, unsigned int subaddress_index) const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    if (!_wallet->get_monero_wallet())
        throw std::runtime_error("monero_wallet_full is not opened");
    double piconero = 0.000000000001;
    // subaddress balance unlocked
    return _wallet->get_monero_wallet()->get_unlocked_balance(account_index, subaddress_index)
           * piconero;
}


QVariantList neroshop::WalletController::getTransfers() const {
    // TODO: make this function async or put in a separate thread
    if (!_wallet.get())
        throw std::runtime_error("neroshop::Wallet is not initialized");
    if (!_wallet->get_monero_wallet())
        throw std::runtime_error("monero_wallet_full is not opened");
    double piconero = 0.000000000001;
    monero_transfer_query transfer_query; // optional
    auto transfers = _wallet->get_monero_wallet()->get_transfers(transfer_query);

    QVariantList transfers_list;

    for (auto transfer : transfers) { /*for(int i = 0; i < transfers.size(); i++) {
        monero_transfer * transfer = transfers[i].get();*/

        QVariantMap transfer_object;
        transfer_object.insert("amount", (transfer->m_amount.get() * piconero));
        transfer_object.insert("account_index", transfer->m_account_index.get()); // obviously account index 0
        transfer_object.insert("is_incoming", transfer->is_incoming().get());
        transfer_object.insert("is_outgoing", transfer->is_outgoing().get());
        monero_tx_wallet * tx_wallet = transfer->m_tx.get();
        ////transfer_object.insert("", tx_wallet->);
        //std::cout << ": " << tx_wallet-> << "\n";
        
        transfers_list.append(transfer_object);
    }
    return transfers_list;
}


void neroshop::WalletController::nodeConnect(const QString& ip, const QString& port, const QString& username, const QString& password) {
    _wallet->daemon_connect_remote(ip.toStdString(),
                                   port.toStdString(),
                                   username.toStdString(),
                                   password.toStdString(),
                                   _wallet.get());
}

void neroshop::WalletController::daemonConnect(const QString& username, const QString& password) {
    _wallet->daemon_connect_local(username.toStdString(), password.toStdString());
}

void neroshop::WalletController::daemonExecute(const QString& daemon_dir, bool confirm_external_bind, bool restricted_rpc, QString data_dir, unsigned int restore_height) {//const {
    _wallet->daemon_open(daemon_dir.toStdString(),
                         confirm_external_bind,
                         restricted_rpc,
                         data_dir.toStdString(),
                         restore_height);
}


double neroshop::WalletController::getSyncPercentage() const {
    std::lock_guard<std::mutex> lock(_wallet->wallet_data_mutex);
    return _wallet->percentage; //wallet->get_sync_percentage();
}

unsigned int neroshop::WalletController::getSyncHeight() const {
    std::lock_guard<std::mutex> lock(_wallet->wallet_data_mutex);
    return _wallet->height; //wallet->get_sync_height();
}

unsigned int neroshop::WalletController::getSyncStartHeight() const {
    std::lock_guard<std::mutex> lock(_wallet->wallet_data_mutex);
    return _wallet->start_height; //wallet->get_sync_start_height();
}

unsigned int neroshop::WalletController::getSyncEndHeight() const {
    std::lock_guard<std::mutex> lock(_wallet->wallet_data_mutex);
    return _wallet->end_height; //wallet->get_sync_end_height();
}

QString neroshop::WalletController::getSyncMessage() const {
    std::lock_guard<std::mutex> lock(_wallet->wallet_data_mutex);
    return QString::fromStdString(
        _wallet->message); //QString::fromStdString(wallet->get_sync_message());
}


void neroshop::WalletController::setNetworkTypeByString(const QString& network_type) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    _wallet->set_network_type_by_string(network_type.toLower().toStdString());
}


bool neroshop::WalletController::isConnectedToDaemon() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    if (!_wallet->get_monero_wallet())
        throw std::runtime_error("monero_wallet_full is not opened");
    return _wallet->get_monero_wallet()->is_connected_to_daemon();
}

bool neroshop::WalletController::isSynced() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    if (!_wallet->get_monero_wallet())
        throw std::runtime_error("monero_wallet_full is not opened");
    return _wallet->get_monero_wallet()->is_synced();
}

bool neroshop::WalletController::isDaemonSynced() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    if (!_wallet->get_monero_wallet())
        throw std::runtime_error("monero_wallet_full is not opened");
    if (!_wallet->get_monero_wallet()->is_connected_to_daemon()) {
        return false;
    }
    return _wallet->get_monero_wallet()
        ->is_daemon_synced(); // will cause crash if wallet is not connected to daemon
}

bool neroshop::WalletController::isOpened() const {
    return (_wallet->get_monero_wallet() != nullptr);
}

bool neroshop::WalletController::fileExists(const QString& filename) const {
    return _wallet->file_exists(filename.toStdString());
}

// Callbacks
void neroshop::WalletController::on_sync_progress(uint64_t height, uint64_t start_height, uint64_t end_height, double percent_done, const std::string& message) {
    std::lock_guard<std::mutex> lock(_wallet->wallet_data_mutex);

    _wallet->percentage = percent_done;
    _wallet->height = height;
    _wallet->start_height = start_height;
    _wallet->end_height = end_height;
    _wallet->message = message;

    //if(percent_done >= 1.0) emit daemonSynced();
}

void neroshop::WalletController::on_new_block (uint64_t height) {
}

void neroshop::WalletController::on_balances_changed(uint64_t new_balance, uint64_t new_unlocked_balance) {
    emit balanceChanged();
}

void neroshop::WalletController::on_output_received(const monero_output_wallet& output) {
    //emit balanceChanged();
    emit transfersChanged();
}

void neroshop::WalletController::on_output_spent (const monero_output_wallet &output) {
    //emit balanceChanged();
    emit transfersChanged();
}

#endif
