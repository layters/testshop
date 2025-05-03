#include "wallet_manager.hpp"

#include "../core/tools/string.hpp"

neroshop::WalletManager::WalletManager(QObject *parent) : QObject(parent)
{
    _wallet = std::make_unique<neroshop::Wallet>(WalletType::Monero); // TODO: replace with: std::make_unique<neroshop::MoneroWallet>();
}

neroshop::WalletManager::~WalletManager() {
    #ifdef NEROSHOP_DEBUG
    std::cout << "wallet manager deleted\n";
    #endif
}

neroshop::Wallet * neroshop::WalletManager::getWallet() const {
    return _wallet.get();
}

// TODO: replace function return type with enum
int neroshop::WalletManager::createRandomWallet(const QString& password, const QString& confirm_pwd, const QString& path) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    auto error = _wallet->create_random(password.toStdString(),
                                        confirm_pwd.toStdString(),
                                        path.toStdString());
    emit walletChanged();
    if(error == static_cast<int>(WalletError::Ok)) emit isOpenedChanged();
    return static_cast<int>(error);
}

int neroshop::WalletManager::restoreFromSeed(const QString& seed, unsigned int restore_height) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    auto error = _wallet->restore_from_seed(seed.toStdString(), restore_height);
    emit walletChanged();
    if(error == static_cast<int>(WalletError::Ok)) emit isOpenedChanged();
    return static_cast<int>(error);
}

int neroshop::WalletManager::restoreFromKeys(const QString& primary_address, const QString& private_view_key, const QString& private_spend_key) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    auto error = _wallet->restore_from_keys(primary_address.toStdString(),
                                               private_view_key.toStdString(),
                                               private_spend_key.toStdString());
    emit walletChanged();
    if(error == static_cast<int>(WalletError::Ok)) emit isOpenedChanged();
    return static_cast<int>(error);
}

int neroshop::WalletManager::open(const QString& path, const QString& password) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    auto error = _wallet->open(path.toStdString(), password.toStdString());
    emit walletChanged();
    if(error == static_cast<int>(WalletError::Ok)) emit isOpenedChanged();
    return static_cast<int>(error);
}

void neroshop::WalletManager::close(bool save) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    _wallet->close(save);
    // set monero_wallet to nullptr so that we know it has been deleted
    _wallet->monero_wallet_obj.reset();
    emit walletChanged();
    emit isOpenedChanged();
}

bool neroshop::WalletManager::verifyPassword(const QString& password) {
    if(!_wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->verify_password(password.toStdString());
}


QVariantMap neroshop::WalletManager::createUniqueSubaddressObject(unsigned int account_idx, const QString & label) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    QVariantMap subaddress_object;
    monero::monero_subaddress subaddress = _wallet->create_subaddress(account_idx,
                                                                      label.toStdString());
    subaddress_object.insert("account_index", subaddress.m_account_index.get());
    subaddress_object.insert("index", subaddress.m_index.get());
    subaddress_object.insert("address", QString::fromStdString(subaddress.m_address.get()));
    subaddress_object.insert("label", QString::fromStdString(subaddress.m_label.get()));
    subaddress_object.insert("balance", (qulonglong(subaddress.m_balance.get()) * PICONERO));
    subaddress_object.insert("unlocked_balance", (qulonglong(subaddress.m_unlocked_balance.get()) * PICONERO));
    subaddress_object.insert("num_unspent_outputs", qulonglong(subaddress.m_num_unspent_outputs.get()));
    subaddress_object.insert("is_used", subaddress.m_is_used.get());
    subaddress_object.insert("num_blocks_to_unlock", qulonglong(subaddress.m_num_blocks_to_unlock.get())); // uint64_t is an unsigned long long so we have to convert it into a qulonglong
    // Usage: console.log("subaddress: ", (!Wallet.isGenerated()) ? "" : Wallet.createUniqueSubaddressObject(0).address)
    return subaddress_object;
}


void neroshop::WalletManager::transfer(const QString& address, double amount) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    _wallet->transfer(address.toStdString(), amount);
}

void neroshop::WalletManager::transfer(const QVariantList &recipients) {
    if (!_wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    
    std::vector<std::pair<std::string, double>> payment_addresses;
    for (const QVariant &recipient : recipients) {
        QVariantMap recipientMap = recipient.toMap();
        QString address = recipientMap["address"].toString();
        double amount = recipientMap["amount"].toDouble();
        payment_addresses.push_back(std::make_pair(address.toStdString(), amount));
    }
    
    _wallet->transfer(payment_addresses);
}

QString neroshop::WalletManager::signMessage(const QString& message) const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return QString::fromStdString(
        _wallet->sign_message(message.toStdString(),
                              monero_message_signature_type::SIGN_WITH_SPEND_KEY));
}

bool neroshop::WalletManager::verifyMessage(const QString& message, const QString& signature) const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->verify_message(message.toStdString(), signature.toStdString());
}

QString neroshop::WalletManager::makeUri(const QString& address, double tx_amount, const QString& tx_description, const QString& recipient_name) const {
    if(!_wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    return QString::fromStdString(_wallet->make_uri(address.toStdString(), tx_amount, tx_description.toStdString(), recipient_name.toStdString()));
}

QVariantMap neroshop::WalletManager::parseUriToObject(const QString& uri) const {
    if(!_wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    QVariantMap uri_object;
    std::string address;
    double tx_amount;
    std::string tx_description;
    std::string recipient_name;
    
    Wallet::parse_uri(uri.toStdString(), address, tx_amount, tx_description, recipient_name);
    
    uri_object.insert("address", QString::fromStdString(address));
    uri_object.insert("tx_amount", tx_amount);
    uri_object.insert("tx_description", QString::fromStdString(tx_description));
    uri_object.insert("recipient_name", QString::fromStdString(recipient_name));
    
    return uri_object;
}

int neroshop::WalletManager::getWalletType() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return static_cast<int>(_wallet->get_wallet_type());
}

int neroshop::WalletManager::getNetworkType() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return static_cast<int>(_wallet->get_wallet_network_type());
}

QString neroshop::WalletManager::getNetworkTypeString() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return QString::fromStdString(_wallet->get_wallet_network_type_as_string());
}

QString neroshop::WalletManager::getNetworkPort() const {
    if (!_wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    return QString::fromStdString(_wallet->get_network_port());
}

QString neroshop::WalletManager::getSeed() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    /*if (!_wallet->get_monero_wallet())
        return "";*/
    return QString::fromStdString(_wallet->get_seed());
}

QStringList neroshop::WalletManager::getSeedList() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList seed_phrase = QString::fromStdString(_wallet->get_seed())
                                  .split(' ');
    return seed_phrase;
}

QString neroshop::WalletManager::getSeedLanguage() const {
    if (!_wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    return QString::fromStdString(_wallet->get_seed_language());
}

QStringList neroshop::WalletManager::getSeedLanguages() const {
    if (!_wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList seed_languages;
    for(const auto& lang : _wallet->get_seed_languages()) {
        seed_languages.append(QString::fromStdString(lang));
    }
    return seed_languages;
}

QString neroshop::WalletManager::getPrimaryAddress() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return QString::fromStdString(_wallet->get_primary_address());
}

QStringList neroshop::WalletManager::getAddressesAll() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList addresses;
    for (auto address : _wallet->get_addresses_all(0)) {
        addresses << QString::fromStdString(address.m_address.get());//std::cout << address << std::endl;
    }
    return addresses;
}

QStringList neroshop::WalletManager::getAddressesUsed() const{
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList addresses;
    for (auto address : _wallet->get_addresses_used(0)) {
        addresses << QString::fromStdString(address.m_address.get());//std::cout << address << " (used)" << std::endl;
    }
    return addresses;    
}

QStringList neroshop::WalletManager::getAddressesUnused() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    QStringList addresses;
    for (auto address : _wallet->get_addresses_unused(0)) {
        addresses << QString::fromStdString(address.m_address.get());//std::cout << address << std::endl;
    }
    return addresses;    
}

double neroshop::WalletManager::getBalanceLocked() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->get_balance();
}

double neroshop::WalletManager::getBalanceLocked(unsigned int account_index) const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->get_balance(account_index);
}

double neroshop::WalletManager::getBalanceLocked(unsigned int account_index, unsigned int subaddress_index) const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->get_balance(account_index, subaddress_index);
}

double neroshop::WalletManager::getBalanceUnlocked() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->get_unlocked_balance();
}

double neroshop::WalletManager::getBalanceUnlocked(unsigned int account_index) const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->get_unlocked_balance(account_index);
}

double neroshop::WalletManager::getBalanceUnlocked(unsigned int account_index, unsigned int subaddress_index) const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->get_unlocked_balance(account_index, subaddress_index);
}


QVariantList neroshop::WalletManager::getTransfers() const {
    if (!_wallet.get()) throw std::runtime_error("neroshop::Wallet is not initialized");
    if (!_wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    // TODO: make this function async or put in a separate thread
    std::packaged_task<QVariantList(void)> get_transfers_task([this]() -> QVariantList {
        monero_transfer_query transfer_query; // optional
        auto transfers = _wallet->get_monero_wallet()->get_transfers(transfer_query);
        //auto txs = _wallet->get_monero_wallet()->get_txs();//(transfer_query);

        QVariantList transfers_list;

        for (auto transfer : transfers) { /*for(int i = 0; i < transfers.size(); i++) {
            monero_transfer * transfer = transfers[i].get();*/

            QVariantMap transfer_object;
            transfer_object.insert("amount", (transfer->m_amount.get() * PICONERO));
            transfer_object.insert("account_index", transfer->m_account_index.get()); // obviously account index 0
            transfer_object.insert("is_incoming", transfer->is_incoming().get());
            transfer_object.insert("is_outgoing", transfer->is_outgoing().get());
            monero_tx_wallet * tx_wallet = transfer->m_tx.get(); // refer to: https://woodser.github.io/monero-cpp/doxygen/structmonero_1_1monero__tx__wallet.html
            ////transfer_object.insert("", tx_wallet->);
            //std::cout << ": " << tx_wallet-> << "\n";
            //--------------------------------------------------------
            rapidjson::Document doc;
            doc.SetObject(); // Set it as an empty object
            rapidjson::Value value = tx_wallet->to_rapidjson_val(doc.GetAllocator());
            
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            value.Accept(writer);

            std::string jsonStr = buffer.GetString();
            doc.Parse(jsonStr.c_str());
            //--------------------------------------------------------
            if (doc.HasMember("fee") && doc["fee"].IsInt()) {
                uint64_t fee = doc["fee"].GetInt();
                std::string feeStr = neroshop::string_tools::precision((fee * PICONERO), 12);
                transfer_object.insert("fee", QString::fromStdString(feeStr));
            }
            if (doc.HasMember("hash") && doc["hash"].IsString()) {
                std::string txId = doc["hash"].GetString();
                transfer_object.insert("tx_id", QString::fromStdString(txId));
                transfer_object.insert("hash", QString::fromStdString(txId));
            }

            // TODO: get block height, note, and timestamp
        
            transfers_list.append(transfer_object);
        }
        return transfers_list;
    });
    
    std::future<QVariantList> future_result = get_transfers_task.get_future();
    // move the task (function) to a separate thread to prevent blocking of the main thread
    std::thread worker(std::move(get_transfers_task));
    worker.detach(); // join may block but detach won't
    QVariantList transfers_result = future_result.get();
    
    return transfers_result;
}


void neroshop::WalletManager::nodeConnect(const QString& ip, const QString& port, const QString& username, const QString& password) {
    if (!_wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    _wallet->daemon_connect_remote(ip.toStdString(),
                                   port.toStdString(),
                                   username.toStdString(),
                                   password.toStdString(),
                                   this);//_wallet.get());
}

void neroshop::WalletManager::daemonConnect(const QString& username, const QString& password) {
    if (!_wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    _wallet->daemon_connect_local(username.toStdString(), password.toStdString());
}

void neroshop::WalletManager::daemonExecute(const QString& daemon_dir, bool confirm_external_bind, bool restricted_rpc, QString data_dir, unsigned int restore_height) {//const {
    if (!_wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    _wallet->daemon_open(daemon_dir.toStdString(),
                         confirm_external_bind,
                         restricted_rpc,
                         data_dir.toStdString(),
                         restore_height);
}


double neroshop::WalletManager::getSyncPercentage() const {
    std::lock_guard<std::mutex> lock(_wallet->wallet_data_mutex);
    return _wallet->percentage; //wallet->get_sync_percentage();
}

unsigned int neroshop::WalletManager::getSyncHeight() const {
    std::lock_guard<std::mutex> lock(_wallet->wallet_data_mutex);
    return _wallet->height; //wallet->get_sync_height();
}

unsigned int neroshop::WalletManager::getSyncStartHeight() const {
    std::lock_guard<std::mutex> lock(_wallet->wallet_data_mutex);
    return _wallet->start_height; //wallet->get_sync_start_height();
}

unsigned int neroshop::WalletManager::getSyncEndHeight() const {
    std::lock_guard<std::mutex> lock(_wallet->wallet_data_mutex);
    return _wallet->end_height; //wallet->get_sync_end_height();
}

QString neroshop::WalletManager::getSyncMessage() const {
    std::lock_guard<std::mutex> lock(_wallet->wallet_data_mutex);
    return QString::fromStdString(
        _wallet->message); //QString::fromStdString(wallet->get_sync_message());
}

void neroshop::WalletManager::setWalletType(unsigned int wallet_type) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    _wallet->set_wallet_type(static_cast<WalletType>(wallet_type));
}

void neroshop::WalletManager::setNetworkTypeByString(const QString& network_type) {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    _wallet->set_network_type_by_string(network_type.toLower().toStdString());
}


bool neroshop::WalletManager::isConnectedToDaemon() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->is_connected_to_daemon();
}

bool neroshop::WalletManager::isSynced() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->is_synced();
}

bool neroshop::WalletManager::isDaemonSynced() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->is_daemon_synced();
}

bool neroshop::WalletManager::isOpened() const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->is_opened();
}

bool neroshop::WalletManager::fileExists(const QString& filename) const {
    if (!_wallet)
        throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->file_exists(filename.toStdString());
}

bool neroshop::WalletManager::isValidAddress(const QString& address) const {
    if (!_wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->is_valid_address(address.toStdString());
}

bool neroshop::WalletManager::isValidOpenAliasAddress(const QString& address) const {
    if (!_wallet) throw std::runtime_error("neroshop::Wallet is not initialized");
    return _wallet->is_valid_openalias_address(address.toStdString());
}

// Callbacks
void neroshop::WalletManager::on_sync_progress(uint64_t height, uint64_t start_height, uint64_t end_height, double percent_done, const std::string& message) {
    std::lock_guard<std::mutex> lock(_wallet->wallet_data_mutex);

    _wallet->percentage = percent_done;
    _wallet->height = height;
    _wallet->start_height = start_height;
    _wallet->end_height = end_height;
    _wallet->message = message;

    //if(percent_done >= 1.0) emit daemonSynced();
}

void neroshop::WalletManager::on_new_block (uint64_t height) {
}

void neroshop::WalletManager::on_balances_changed(uint64_t new_balance, uint64_t new_unlocked_balance) {
    emit balanceChanged();
}

void neroshop::WalletManager::on_output_received(const monero_output_wallet& output) {
    emit transfersChanged();
}

void neroshop::WalletManager::on_output_spent (const monero_output_wallet &output) {
    emit transfersChanged();
}

