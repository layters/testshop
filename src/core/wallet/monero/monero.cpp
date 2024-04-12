#include "monero.hpp"

#include "../../tools/string.hpp"

neroshop::MoneroWallet::MoneroWallet() : Wallet(WalletType::Monero)
{}
//-------------------------------------------------------
neroshop::MoneroWallet::~MoneroWallet() {}
//-------------------------------------------------------
//-------------------------------------------------------
int neroshop::MoneroWallet::create_random(const std::string& password, const std::string& confirm_pwd, const std::string& path) {
    if(confirm_pwd != password) {
        std::cerr << "\033[1;91mWallet passwords do not match\033[0m\n";
        return static_cast<int>(WalletError::PasswordsDoNotMatch);
    }
    
    if(monero::monero_wallet_full::wallet_exists(path + ".keys")) {//if(std::filesystem::is_regular_file(path + ".keys")) {
        std::cerr << "\033[1;91mWallet file with the same name already exists\033[0m\n";
        return static_cast<int>(WalletError::AlreadyExists);
    }
    
    monero::monero_wallet_config wallet_config_obj;
    wallet_config_obj.m_path = path;
    wallet_config_obj.m_password = password;
    wallet_config_obj.m_network_type = static_cast<monero::monero_network_type>(this->network_type);
    
    monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet (wallet_config_obj, nullptr));
    if(monero_wallet_obj.get()) std::cout << "\033[1;35m" << "created wallet \"" << path << ".keys\"" << "\033[0m" << std::endl;
    
    password_hash = sign_message(password, monero_message_signature_type::SIGN_WITH_SPEND_KEY);
    
    return static_cast<int>(WalletError::Ok);
}
//-------------------------------------------------------
int neroshop::MoneroWallet::create_from_seed(const std::string& seed, const std::string& password, const std::string& confirm_pwd, const std::string& path) {
    if(confirm_pwd != password) {
        std::cerr << "\033[1;91mWallet passwords do not match\033[0m\n";
        return static_cast<int>(WalletError::PasswordsDoNotMatch);
    }    

    if(monero::monero_wallet_full::wallet_exists(path + ".keys")) {
        std::cerr << "\033[1;91mWallet file with the same name already exists\033[0m\n";
        return static_cast<int>(WalletError::AlreadyExists);
    }
    
    monero::monero_wallet_config wallet_config_obj;
    wallet_config_obj.m_path = path;
    wallet_config_obj.m_password = password;
    wallet_config_obj.m_network_type = static_cast<monero::monero_network_type>(this->network_type);
    wallet_config_obj.m_seed = seed;
    
    monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet (wallet_config_obj, nullptr));
    if(!monero_wallet_obj.get()) return static_cast<int>(WalletError::IsNotOpened);
    std::cout << "\033[1;35m" << "created wallet \"" << path << ".keys\" (from seed)" << "\033[0m" << std::endl;
    
    password_hash = sign_message(password, monero_message_signature_type::SIGN_WITH_SPEND_KEY);
        
    return static_cast<int>(WalletError::Ok);
}
//-------------------------------------------------------
int neroshop::MoneroWallet::create_from_keys(const std::string& primary_address, const std::string& view_key, const std::string& spend_key, const std::string& password, const std::string &confirm_pwd, const std::string& path) {
    if(confirm_pwd != password) {
        std::cerr << "\033[1;91mWallet passwords do not match\033[0m\n";
        return static_cast<int>(WalletError::PasswordsDoNotMatch);
    }  
    
    if(monero::monero_wallet_full::wallet_exists(path + ".keys")) {
        std::cerr << "\033[1;91mWallet file with the same name already exists\033[0m\n";
        return static_cast<int>(WalletError::AlreadyExists);
    }
    
    monero::monero_wallet_config wallet_config_obj;
    wallet_config_obj.m_path = path;
    wallet_config_obj.m_password = password;
    wallet_config_obj.m_network_type = static_cast<monero::monero_network_type>(this->network_type);
    wallet_config_obj.m_primary_address = primary_address;
    wallet_config_obj.m_private_view_key = view_key;
    wallet_config_obj.m_private_spend_key = spend_key; // To restore a view-only wallet, leave the spend key blank
    
    monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet (wallet_config_obj, nullptr));
    if(!monero_wallet_obj.get()) return static_cast<int>(WalletError::IsNotOpened);
    std::cout << "\033[1;35m" << "created wallet \"" << path << ".keys\" (from keys)" << "\033[0m" << std::endl;
    
    password_hash = sign_message(password, monero_message_signature_type::SIGN_WITH_SPEND_KEY);
        
    return static_cast<int>(WalletError::Ok);
}
//-------------------------------------------------------
//-------------------------------------------------------
int neroshop::MoneroWallet::restore_from_seed(const std::string& seed, uint64_t restore_height) 
{
    monero::monero_wallet_config wallet_config_obj;
    wallet_config_obj.m_path = ""; // set path to "" for an in-memory wallet
    wallet_config_obj.m_password = "";
    wallet_config_obj.m_network_type = static_cast<monero::monero_network_type>(this->network_type);
    wallet_config_obj.m_seed = seed;
    wallet_config_obj.m_restore_height = restore_height;
    
    try {
        monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet (wallet_config_obj, nullptr));
    } catch (const std::exception& e) {
        std::string error_msg = e.what();
        std::cerr << "\033[1;91m" << error_msg << "\033[0m\n";
        if(neroshop::string::contains(error_msg, "Invalid mnemonic")) {
            return static_cast<int>(WalletError::InvalidMnemonic);
        }
    }
    if(!monero_wallet_obj.get()) return static_cast<int>(WalletError::IsNotOpened);
    std::cout << "\033[1;35m" << "restored in-memory wallet (from seed)" << "\033[0m" << std::endl;
    
    password_hash = sign_message(wallet_config_obj.m_password.get(), monero_message_signature_type::SIGN_WITH_SPEND_KEY);
    
    return static_cast<int>(WalletError::Ok);
}
//-------------------------------------------------------
int neroshop::MoneroWallet::restore_from_keys(const std::string& primary_address, const std::string& view_key, const std::string& spend_key)
{
    // Check validity of primary address
    if(!monero_utils::is_valid_address(primary_address, static_cast<monero::monero_network_type>(this->network_type))) {
        std::cerr << "\033[1;91mInvalid Monero address\033[0m\n";
        return static_cast<int>(WalletError::InvalidAddress);
    }
    
    monero::monero_wallet_config wallet_config_obj;
    wallet_config_obj.m_path = ""; // set path to "" for an in-memory wallet
    wallet_config_obj.m_password = "";
    wallet_config_obj.m_network_type = static_cast<monero::monero_network_type>(this->network_type);
    wallet_config_obj.m_primary_address = primary_address;
    wallet_config_obj.m_private_view_key = view_key;
    wallet_config_obj.m_private_spend_key = spend_key;
    
    monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet (wallet_config_obj, nullptr));    
    if(!monero_wallet_obj.get()) return static_cast<int>(WalletError::IsNotOpened);
    std::cout << "\033[1;35m" << "restored in-memory wallet (from keys)" << "\033[0m" << std::endl;
    
    password_hash = sign_message(wallet_config_obj.m_password.get(), monero_message_signature_type::SIGN_WITH_SPEND_KEY);
    
    return static_cast<int>(WalletError::Ok);
}
//-------------------------------------------------------
//-------------------------------------------------------
int neroshop::MoneroWallet::open(const std::string& path, const std::string& password) { 
    if(!monero::monero_wallet_full::wallet_exists(path)) { 
        return static_cast<int>(WalletError::DoesNotExist);
    }

    try {
        monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero::monero_wallet_full::open_wallet(path, password, static_cast<monero::monero_network_type>(this->network_type)));
    } catch (const std::exception& e) {
        std::string error_msg = e.what();
        std::cerr << "\033[1;91m" << error_msg << "\033[0m\n";//tools::error::invalid_password
        if(neroshop::string::contains(error_msg, "wallet cannot be opened as")) {
            return static_cast<int>(WalletError::BadNetworkType);
        } else if(neroshop::string::contains(error_msg, "invalid password")) {
            return static_cast<int>(WalletError::WrongPassword);
        } else if(neroshop::string::contains(error_msg, "Invalid decimal point specification")) {
            return static_cast<int>(WalletError::BadWalletType);
        } else {
            return static_cast<int>(WalletError::IsOpenedByAnotherProgram);
        }
    }
    if(!monero_wallet_obj.get()) { return static_cast<int>(WalletError::IsNotOpened); }
    std::cout << "\033[1;35m" << "opened wallet \"" << path << "\"\033[0m" << std::endl;
    
    password_hash = sign_message(password, monero_message_signature_type::SIGN_WITH_SPEND_KEY);
        
    return static_cast<int>(WalletError::Ok);
}
//-------------------------------------------------------
void neroshop::MoneroWallet::close(bool save) {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    monero_wallet_obj->close(save);
}
//-------------------------------------------------------
std::string neroshop::MoneroWallet::sign_message(const std::string& message, monero_message_signature_type signature_type) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->sign_message(message, signature_type, 0, 0);
}
//-------------------------------------------------------
bool neroshop::MoneroWallet::verify_message(const std::string& message, const std::string& signature) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->verify_message(message, monero_wallet_obj.get()->get_primary_address(), signature).m_is_good;
}
//-------------------------------------------------------
//-------------------------------------------------------
void neroshop::MoneroWallet::transfer(const std::string& address, double amount) {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    if(!monero_wallet_obj.get()->is_synced()) throw std::runtime_error("wallet is not synced with a daemon");
    std::packaged_task<void(void)> transfer_task([this, address, amount]() -> void {
    // Check if address is valid
    if(!monero_utils::is_valid_address(address, monero_wallet_obj->get_network_type())) {
        std::cerr << "\033[1;91mMonero address is invalid" << "\033[0m\n"; return;
    }
    // Convert monero to piconero
    uint64_t monero_to_piconero = amount / PICONERO; //std::cout << neroshop::string::precision(amount, 12) << " xmr to piconero: " << monero_to_piconero << "\n";
    // TODO: for the 2-of-3 escrow system, take 0.5% of order total in piconeros
    // Check if amount is zero or too low
    if((amount < PICONERO) || (monero_to_piconero == 0)) {
        std::cerr << "\033[1;91mNothing to send (amount is zero)" << "\033[0m\n"; return;
    }
    // Check if balance is sufficient
    std::cout << "Wallet balance (spendable): " << monero_wallet_obj->get_unlocked_balance() << " (picos)\n";
    std::cout << "Amount to send: " << monero_to_piconero << " (picos)\n";
    if(monero_wallet_obj->get_unlocked_balance() < monero_to_piconero) {
        std::cerr << "\033[1;91mWallet balance is insufficient" << "\033[0m\n"; return;
    }
    // Send funds from this wallet to the specified address
    monero_tx_config config; // Configures a transaction to send, sweep, or create a payment URI.
    config.m_account_index = 0; // withdraw funds from account at index 0
    config.m_address = address; // address that will be receiving the funds
    config.m_amount = monero_to_piconero;
    config.m_relay = true;
    // Sweep unlocked balance?
    if(monero_wallet_obj->get_unlocked_balance() == monero_to_piconero) {
        std::cout << "\033[1;37mSweeping unlocked balance ...\033[0m\n";
        config.m_amount = boost::none;
        monero_wallet_obj->sweep_unlocked(config);return;
    }    
    // Create the transaction
    std::shared_ptr<monero_tx_wallet> sent_tx = monero_wallet_obj->create_tx(config);
    bool in_pool = sent_tx->m_in_tx_pool.get();  // true
    // Get tx fee and hash    
    uint64_t fee = sent_tx->m_fee.get(); // "Are you sure you want to send ...?"
    std::cout << "Estimated fee: " << (fee * PICONERO) << "\n";
    //uint64_t deducted_amount = (monero_to_piconero + fee);
    std::string tx_hash = monero_wallet_obj->relay_tx(*sent_tx); // recipient receives notification within 5 seconds    
    std::cout << "Tx hash: " << tx_hash << "\n";
    monero_utils::free(sent_tx);
    });
    
    std::future<void> future_result = transfer_task.get_future();
    // move the task (function) to a separate thread to prevent blocking of the main thread
    std::thread worker(std::move(transfer_task));
    worker.detach(); // join may block but detach won't//void transfer_result = future_result.get();
}
//-------------------------------------------------------
void neroshop::MoneroWallet::transfer(const std::vector<std::pair<std::string, double>>& payment_addresses) { // untested
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    if(!monero_wallet_obj.get()->is_synced()) throw std::runtime_error("wallet is not synced with a daemon");
    
    monero_tx_config tx_config = monero_tx_config();
    tx_config.m_account_index = 0; // withdraw funds from this account
    tx_config.m_relay = false; // create transaction and relay to the network if true
    
    // Calculate the total amount owed
    double total_amount = 0.000000000000;
    for (const auto& address : payment_addresses) {
        total_amount += address.second;
    }
    std::cout << "Total amount to pay: " << total_amount << " (xmr)\n";
    // Check if balance is sufficient
    uint64_t total_to_piconero = total_amount / PICONERO;
    std::cout << "Wallet balance (spendable): " << monero_wallet_obj->get_unlocked_balance() << " (picos)\n";
    std::cout << "Amount to send: " << total_to_piconero << " (picos)\n";
    if(monero_wallet_obj->get_unlocked_balance() < total_to_piconero) {
        std::cerr << "\033[1;91mWallet balance is insufficient" << "\033[0m\n"; return;
    }
    // Add each destination
    std::vector<std::shared_ptr<monero_destination>> destinations; // specify the recipients and their amounts
    for(const auto& address : payment_addresses) {
        // Check if address is valid
        if(!monero_utils::is_valid_address(address.first, monero_wallet_obj->get_network_type())) {
            std::cerr << "\033[1;91m" << address.first + " is not a valid Monero address" << "\033[0m\n";
            continue; // skip to the next address
        }
        // Convert monero to piconero
        uint64_t monero_to_piconero = address.second / PICONERO; //std::cout << neroshop::string::precision(address.second, 12) << " xmr to piconero: " << monero_to_piconero << "\n";
        destinations.push_back(std::make_shared<monero_destination>(address.first, monero_to_piconero));
        // Print address and amount
        std::cout << "Address: " << address.first << ", Amount: " << address.second << std::endl;
    }
    tx_config.m_destinations = destinations;
    
    // Create the transaction, confirm with the user, and relay to the network
    std::shared_ptr<monero_tx_wallet> created_tx = monero_wallet_obj->create_tx(tx_config);
    uint64_t fee = created_tx->m_fee.get(); // "Are you sure you want to send ...?"
    monero_wallet_obj->relay_tx(*created_tx); // recipient receives notification within 5 seconds
    monero_utils::free(created_tx);
}
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
std::string neroshop::MoneroWallet::get_primary_address() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_primary_address(); // same as: monero_wallet_obj->get_account(0, true).m_primary_address.get();
}
//-------------------------------------------------------
std::string neroshop::MoneroWallet::get_address(unsigned int index) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_account(0, true).m_subaddresses[index].m_address.get(); // account_idx is 0
}
//-------------------------------------------------------
//-------------------------------------------------------
uint64_t neroshop::MoneroWallet::get_balance_raw() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_balance(); // get wallet balance
}
//-------------------------------------------------------
uint64_t neroshop::MoneroWallet::get_balance_raw(unsigned int account_index) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_balance(account_index); // get balance from account
}
//-------------------------------------------------------
uint64_t neroshop::MoneroWallet::get_balance_raw(unsigned int account_index, unsigned int subaddress_index) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_balance(account_index, subaddress_index); // get balance from subaddress
}
//-------------------------------------------------------
uint64_t neroshop::MoneroWallet::get_unlocked_balance_raw() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_unlocked_balance(); // get wallet unlocked balance
}
//-------------------------------------------------------
uint64_t neroshop::MoneroWallet::get_unlocked_balance_raw(unsigned int account_index) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_unlocked_balance(account_index); // get unlocked balance from account      
}
//-------------------------------------------------------
uint64_t neroshop::MoneroWallet::get_unlocked_balance_raw(unsigned int account_index, unsigned int subaddress_index) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_unlocked_balance(account_index, subaddress_index); // get unlocked balance from subaddress
}
//-------------------------------------------------------
//-------------------------------------------------------
double neroshop::MoneroWallet::get_balance() const {
    auto raw_balance = get_balance_raw();
    return raw_balance * PICONERO;
}
//-------------------------------------------------------
double neroshop::MoneroWallet::get_balance(unsigned int account_index) const {
    auto raw_balance = get_balance_raw(account_index);
    return raw_balance * PICONERO;
}
//-------------------------------------------------------
double neroshop::MoneroWallet::get_balance(unsigned int account_index, unsigned int subaddress_index) const {
    auto raw_balance = get_balance_raw(account_index, subaddress_index);
    return raw_balance * PICONERO;
}
//-------------------------------------------------------
double neroshop::MoneroWallet::get_unlocked_balance() const {
    auto raw_unlocked_balance = get_unlocked_balance_raw();
    return raw_unlocked_balance * PICONERO;
}
//-------------------------------------------------------
double neroshop::MoneroWallet::get_unlocked_balance(unsigned int account_index) const {
    auto raw_unlocked_balance = get_unlocked_balance_raw(account_index);
    return raw_unlocked_balance * PICONERO;
}
//-------------------------------------------------------
double neroshop::MoneroWallet::get_unlocked_balance(unsigned int account_index, unsigned int subaddress_index) const {
    auto raw_unlocked_balance = get_unlocked_balance_raw(account_index, subaddress_index);
    return raw_unlocked_balance * PICONERO;
}
//-------------------------------------------------------
//-------------------------------------------------------
std::string neroshop::MoneroWallet::get_private_view_key() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_private_view_key();
}
//-------------------------------------------------------
std::string neroshop::MoneroWallet::get_public_view_key() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_public_view_key();
}
//-------------------------------------------------------
std::pair<std::string, std::string> neroshop::MoneroWallet::get_view_keys() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return std::make_pair(monero_wallet_obj->get_private_view_key(), monero_wallet_obj->get_public_view_key());
}
//-------------------------------------------------------
//-------------------------------------------------------
std::string neroshop::MoneroWallet::get_private_spend_key() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_private_spend_key();
}
//-------------------------------------------------------
std::string neroshop::MoneroWallet::get_public_spend_key() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_public_spend_key();
}
//-------------------------------------------------------
std::pair<std::string, std::string> neroshop::MoneroWallet::get_spend_keys() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return std::make_pair(monero_wallet_obj->get_private_spend_key(), monero_wallet_obj->get_public_spend_key());
}
//-------------------------------------------------------
//-------------------------------------------------------
std::string neroshop::MoneroWallet::get_seed() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_seed();
}
//-------------------------------------------------------
//-------------------------------------------------------
std::string neroshop::MoneroWallet::get_path() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->monero_wallet_full::get_path(); // returns the path of this wallet's file on disk (without the .keys ext)
}
//-------------------------------------------------------
//-------------------------------------------------------
unsigned int neroshop::MoneroWallet::get_daemon_height() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_daemon_height();
}
//-------------------------------------------------------
unsigned int neroshop::MoneroWallet::get_height() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_height();
}
//-------------------------------------------------------
unsigned int neroshop::MoneroWallet::get_height_by_date(int year, int month, int day) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_height_by_date(year, month, day);
}
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
bool neroshop::MoneroWallet::is_opened() const {
    return (monero_wallet_obj != nullptr);
}
//-------------------------------------------------------
bool neroshop::MoneroWallet::is_connected_to_daemon() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->is_connected_to_daemon();
}
//-------------------------------------------------------
bool neroshop::MoneroWallet::is_synced() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->is_synced();
}
//-------------------------------------------------------
bool neroshop::MoneroWallet::is_daemon_synced() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    if(!monero_wallet_obj->is_connected_to_daemon()) {
        return false;
    }
    return monero_wallet_obj->is_daemon_synced(); // will cause crash if wallet is not connected to daemon
}
//-------------------------------------------------------
//-------------------------------------------------------
bool neroshop::MoneroWallet::file_exists(const std::string& filename) const {
    return monero::monero_wallet_full::wallet_exists(filename + ".keys");
}
//-------------------------------------------------------
bool neroshop::MoneroWallet::is_valid_address(const std::string& address) const {
    auto network_type = get_wallet_network_type();
    return monero_utils::is_valid_address(address, static_cast<monero::monero_network_type>(network_type));
}
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
