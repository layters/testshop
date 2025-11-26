#include "monero.hpp"

#include "../../tools/string.hpp"
#include "../../settings.hpp" // language

namespace neroshop {

MoneroWallet::MoneroWallet() : Wallet(WalletType::Monero) {}
//-------------------------------------------------------
MoneroWallet::~MoneroWallet() {}
//-------------------------------------------------------
//-------------------------------------------------------
int MoneroWallet::create_random(const std::string& password, const std::string& confirm_pwd, const std::string& path) {
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
    wallet_config_obj.m_network_type = static_cast<monero::monero_network_type>(Wallet::network_type);
    std::string json_str = neroshop::load_json();
    rapidjson::Document settings;
    wallet_config_obj.m_language = "English";  // default language
    settings.Parse(json_str.c_str());
    if (!settings.HasParseError() && settings.IsObject()) {
        if (settings.HasMember("monero") && settings["monero"].IsObject()) {
            const rapidjson::Value& monero_obj = settings["monero"];

            if (monero_obj.HasMember("wallet") && monero_obj["wallet"].IsObject()) {
                const rapidjson::Value& wallet_obj = monero_obj["wallet"];

                if (wallet_obj.HasMember("seed_language") && wallet_obj["seed_language"].IsString()) {
                    wallet_config_obj.m_language = wallet_obj["seed_language"].GetString();
                }
            }
        }
    }
    
    monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet (wallet_config_obj, nullptr));
    if(!monero_wallet_obj.get()) return static_cast<int>(WalletError::IsNotOpened);
    std::cout << "\033[1;35m" << "created wallet \"" << path << ".keys\"" << "\033[0m" << std::endl;
    
    password_hash = sign_message(password, monero_message_signature_type::SIGN_WITH_SPEND_KEY);
    
    return static_cast<int>(WalletError::Ok);
}
//-------------------------------------------------------
int MoneroWallet::create_from_seed(const std::string& seed, const std::string& password, const std::string& confirm_pwd, const std::string& path) {
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
    wallet_config_obj.m_network_type = static_cast<monero::monero_network_type>(Wallet::network_type);
    wallet_config_obj.m_seed = seed;
    
    monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet (wallet_config_obj, nullptr));
    if(!monero_wallet_obj.get()) return static_cast<int>(WalletError::IsNotOpened);
    std::cout << "\033[1;35m" << "created wallet \"" << path << ".keys\" (from seed)" << "\033[0m" << std::endl;
    
    password_hash = sign_message(password, monero_message_signature_type::SIGN_WITH_SPEND_KEY);
        
    return static_cast<int>(WalletError::Ok);
}
//-------------------------------------------------------
int MoneroWallet::create_from_keys(const std::string& primary_address, const std::string& view_key, const std::string& spend_key, const std::string& password, const std::string &confirm_pwd, const std::string& path) {
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
    wallet_config_obj.m_network_type = static_cast<monero::monero_network_type>(Wallet::network_type);
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
int MoneroWallet::restore_from_seed(const std::string& seed, uint64_t restore_height) 
{
    monero::monero_wallet_config wallet_config_obj;
    wallet_config_obj.m_path = ""; // Set path to "" for an in-memory wallet
    wallet_config_obj.m_password = "";
    wallet_config_obj.m_network_type = static_cast<monero::monero_network_type>(Wallet::network_type);
    wallet_config_obj.m_seed = seed;
    wallet_config_obj.m_restore_height = restore_height;
    
    try {
        monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet (wallet_config_obj, nullptr));
    } catch (const std::exception& e) {
        std::string error_msg = e.what();
        std::cerr << "\033[1;91m" << error_msg << "\033[0m\n";
        if(neroshop::string_tools::contains(error_msg, "Invalid mnemonic")) {
            return static_cast<int>(WalletError::InvalidMnemonic);
        }
    }
    if(!monero_wallet_obj.get()) return static_cast<int>(WalletError::IsNotOpened);
    std::cout << "\033[1;35m" << "restored in-memory wallet (from seed)" << "\033[0m" << std::endl;
    
    password_hash = sign_message(wallet_config_obj.m_password.get(), monero_message_signature_type::SIGN_WITH_SPEND_KEY);
    
    return static_cast<int>(WalletError::Ok);
}
//-------------------------------------------------------
int MoneroWallet::restore_from_keys(const std::string& primary_address, const std::string& view_key, const std::string& spend_key)
{
    // Check validity of primary address
    if(!monero_utils::is_valid_address(primary_address, static_cast<monero::monero_network_type>(Wallet::network_type))) {
        std::cerr << "\033[1;91mInvalid Monero address\033[0m\n";
        return static_cast<int>(WalletError::InvalidAddress);
    }
    
    monero::monero_wallet_config wallet_config_obj;
    wallet_config_obj.m_path = ""; // Set path to "" for an in-memory wallet
    wallet_config_obj.m_password = "";
    wallet_config_obj.m_network_type = static_cast<monero::monero_network_type>(Wallet::network_type);
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
int MoneroWallet::open(const std::string& path, const std::string& password) { 
    if(!monero::monero_wallet_full::wallet_exists(path)) { 
        return static_cast<int>(WalletError::DoesNotExist);
    }

    try {
        monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero::monero_wallet_full::open_wallet(path, password, static_cast<monero::monero_network_type>(Wallet::network_type)));
    } catch (const std::exception& e) {
        std::string error_msg = e.what();
        std::cerr << "\033[1;91m" << error_msg << "\033[0m\n";//tools::error::invalid_password
        if(neroshop::string_tools::contains(error_msg, "wallet cannot be opened as")) {
            return static_cast<int>(WalletError::BadNetworkType);
        } else if(neroshop::string_tools::contains(error_msg, "invalid password")) {
            return static_cast<int>(WalletError::WrongPassword);
        } else if(neroshop::string_tools::contains(error_msg, "Invalid decimal point specification")) {
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
void MoneroWallet::close(bool save) {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    monero_wallet_obj->close(save);
}
//-------------------------------------------------------
//-------------------------------------------------------
bool MoneroWallet::change_password(const std::string& old_password, const std::string& new_password) {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    if(!verify_password(old_password)) {
        std::cout << "\033[1;91mOld password is incorrect\033[0m\n";
        return false;
    }
    monero_wallet_obj->change_password(old_password, new_password);
    password_hash = sign_message(new_password, monero_message_signature_type::SIGN_WITH_SPEND_KEY);
    return true;
}
//-------------------------------------------------------
//-------------------------------------------------------
std::string MoneroWallet::sign_message(const std::string& message, monero_message_signature_type signature_type) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->sign_message(message, signature_type, 0, 0);
}
//-------------------------------------------------------
bool MoneroWallet::verify_message(const std::string& message, const std::string& signature) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->verify_message(message, monero_wallet_obj.get()->get_primary_address(), signature).m_is_good;
}
//-------------------------------------------------------
//-------------------------------------------------------
void MoneroWallet::transfer(const std::string& address, double amount) {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    if(!monero_wallet_obj.get()->is_synced()) throw std::runtime_error("wallet is not synced with a daemon");
    std::packaged_task<void(void)> transfer_task([this, address, amount]() -> void {
    // Check if address is valid
    if(!monero_utils::is_valid_address(address, monero_wallet_obj->get_network_type())) {
        std::cerr << "\033[1;91mMonero address is invalid" << "\033[0m\n"; return;
    }
    // Convert monero to piconero
    uint64_t monero_to_piconero = amount / PICONERO; //std::cout << neroshop::string_tools::precision(amount, 12) << " xmr to piconero: " << monero_to_piconero << "\n";
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
void MoneroWallet::transfer(const std::vector<std::pair<std::string, double>>& payment_addresses) { // untested
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
        uint64_t monero_to_piconero = address.second / PICONERO; //std::cout << neroshop::string_tools::precision(address.second, 12) << " xmr to piconero: " << monero_to_piconero << "\n";
        destinations.push_back(std::make_shared<monero_destination>(address.first, monero_to_piconero));
        // Print address and amount
        std::cout << "Address: " << address.first << ", Amount: " << address.second << std::endl;
    }
    tx_config.m_destinations = destinations;
    if(tx_config.m_destinations.empty()) {
        std::cerr << "\033[1;91mNo destinations for this transfer" << "\033[0m\n";
        return;
    }
    
    // Create the transaction, confirm with the user, and relay to the network
    std::shared_ptr<monero_tx_wallet> created_tx = monero_wallet_obj->create_tx(tx_config);
    uint64_t fee = created_tx->m_fee.get(); // "Are you sure you want to send ...?"
    monero_wallet_obj->relay_tx(*created_tx); // recipient receives notification within 5 seconds
    monero_utils::free(created_tx);
}
//-------------------------------------------------------
void MoneroWallet::transfer(const std::string& uri) {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    if(!monero_wallet_obj.get()->is_synced()) throw std::runtime_error("wallet is not synced with a daemon");
    
    std::string address;
    double tx_amount = 0.000000000000;
    std::string tx_description;
    std::string recipient_name;
    
    parse_uri(uri, address, tx_amount, tx_description, recipient_name);
}
//-------------------------------------------------------
//-------------------------------------------------------
std::string MoneroWallet::make_uri(const std::string& payment_address, double amount, const std::string& description, const std::string& recipient) const {
    bool has_amount = false;
    bool has_recipient = false;
    std::string monero_uri = "monero:";
    if(is_valid_address(payment_address)) {
        monero_uri = monero_uri + payment_address;
    }
    if(amount > std::stod(neroshop::string_tools::precision("0.000000000000", 12))) {
        has_amount = true;
        monero_uri = monero_uri + "?tx_amount=" + neroshop::string_tools::precision(amount, 12);
    }
    if(!recipient.empty()) {
        has_recipient = true;
        std::string recipient_name = neroshop::string_tools::swap_all(recipient, " ", "%20");
        monero_uri = monero_uri + ((has_amount) ? "&recipient_name=" : "?recipient_name=") + recipient_name;
    }
    if(!description.empty()) {
        std::string tx_description = neroshop::string_tools::swap_all(description, " ", "%20");
        monero_uri = monero_uri + ((has_amount || has_recipient) ? "&tx_description=" : "?tx_description=") + tx_description;
    }
    return monero_uri;
}
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
void MoneroWallet::set_network_type(WalletNetworkType network_type) {
    auto current_network_type = get_wallet_network_type();
    if(current_network_type == network_type) {
        return;
    }
    auto network_type_str = get_wallet_network_type_as_string();
    if(monero_wallet_obj.get()) throw std::runtime_error("cannot change " + network_type_str + " wallet network type");
    Wallet::network_type = network_type;
}
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
neroshop::WalletNetworkType MoneroWallet::get_wallet_network_type() const {
    if(!monero_wallet_obj.get()) return Wallet::network_type;
    return static_cast<WalletNetworkType>(monero_wallet_obj->get_network_type());
}
//-------------------------------------------------------
std::string MoneroWallet::get_network_port() const {
    auto wallet_network_type = get_wallet_network_type();
    return WalletNetworkPortMap[wallet_network_type][0];
}
//-------------------------------------------------------
//-------------------------------------------------------
std::string MoneroWallet::get_primary_address() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_primary_address(); // same as: monero_wallet_obj->get_account(0, true).m_primary_address.get();
}
//-------------------------------------------------------
std::string MoneroWallet::get_address(unsigned int index) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_account(0, true).m_subaddresses[index].m_address.get(); // account_idx is 0
}
//-------------------------------------------------------
//-------------------------------------------------------
uint64_t MoneroWallet::get_balance_raw() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_balance(); // get wallet balance
}
//-------------------------------------------------------
uint64_t MoneroWallet::get_balance_raw(unsigned int account_index) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_balance(account_index); // get balance from account
}
//-------------------------------------------------------
uint64_t MoneroWallet::get_balance_raw(unsigned int account_index, unsigned int subaddress_index) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_balance(account_index, subaddress_index); // get balance from subaddress
}
//-------------------------------------------------------
uint64_t MoneroWallet::get_unlocked_balance_raw() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_unlocked_balance(); // get wallet unlocked balance
}
//-------------------------------------------------------
uint64_t MoneroWallet::get_unlocked_balance_raw(unsigned int account_index) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_unlocked_balance(account_index); // get unlocked balance from account      
}
//-------------------------------------------------------
uint64_t MoneroWallet::get_unlocked_balance_raw(unsigned int account_index, unsigned int subaddress_index) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_unlocked_balance(account_index, subaddress_index); // get unlocked balance from subaddress
}
//-------------------------------------------------------
//-------------------------------------------------------
double MoneroWallet::get_balance() const {
    auto raw_balance = get_balance_raw();
    return raw_balance * PICONERO;
}
//-------------------------------------------------------
double MoneroWallet::get_balance(unsigned int account_index) const {
    auto raw_balance = get_balance_raw(account_index);
    return raw_balance * PICONERO;
}
//-------------------------------------------------------
double MoneroWallet::get_balance(unsigned int account_index, unsigned int subaddress_index) const {
    auto raw_balance = get_balance_raw(account_index, subaddress_index);
    return raw_balance * PICONERO;
}
//-------------------------------------------------------
double MoneroWallet::get_unlocked_balance() const {
    auto raw_unlocked_balance = get_unlocked_balance_raw();
    return raw_unlocked_balance * PICONERO;
}
//-------------------------------------------------------
double MoneroWallet::get_unlocked_balance(unsigned int account_index) const {
    auto raw_unlocked_balance = get_unlocked_balance_raw(account_index);
    return raw_unlocked_balance * PICONERO;
}
//-------------------------------------------------------
double MoneroWallet::get_unlocked_balance(unsigned int account_index, unsigned int subaddress_index) const {
    auto raw_unlocked_balance = get_unlocked_balance_raw(account_index, subaddress_index);
    return raw_unlocked_balance * PICONERO;
}
//-------------------------------------------------------
//-------------------------------------------------------
std::string MoneroWallet::get_private_view_key() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_private_view_key();
}
//-------------------------------------------------------
std::string MoneroWallet::get_public_view_key() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_public_view_key();
}
//-------------------------------------------------------
std::pair<std::string, std::string> MoneroWallet::get_view_keys() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return std::make_pair(monero_wallet_obj->get_private_view_key(), monero_wallet_obj->get_public_view_key());
}
//-------------------------------------------------------
//-------------------------------------------------------
std::string MoneroWallet::get_private_spend_key() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_private_spend_key();
}
//-------------------------------------------------------
std::string MoneroWallet::get_public_spend_key() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_public_spend_key();
}
//-------------------------------------------------------
std::pair<std::string, std::string> MoneroWallet::get_spend_keys() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return std::make_pair(monero_wallet_obj->get_private_spend_key(), monero_wallet_obj->get_public_spend_key());
}
//-------------------------------------------------------
//-------------------------------------------------------
std::string MoneroWallet::get_seed() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_seed();
}
//-------------------------------------------------------
std::string MoneroWallet::get_seed_language() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_seed_language();
}
//-------------------------------------------------------
std::vector<std::string> MoneroWallet::get_seed_languages() const {
    return monero::monero_wallet_full::get_seed_languages();
}
//-------------------------------------------------------
//-------------------------------------------------------
std::string MoneroWallet::get_path() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->monero_wallet_full::get_path(); // returns the path of this wallet's file on disk (without the .keys ext)
}
//-------------------------------------------------------
//-------------------------------------------------------
unsigned int MoneroWallet::get_daemon_height() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_daemon_height();
}
//-------------------------------------------------------
unsigned int MoneroWallet::get_height() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_height();
}
//-------------------------------------------------------
unsigned int MoneroWallet::get_height_by_date(int year, int month, int day) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_height_by_date(year, month, day);
}
//-------------------------------------------------------
//-------------------------------------------------------
void * MoneroWallet::get_handle() const {
    return monero_wallet_obj.get();
}
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
bool MoneroWallet::is_opened() const {
    return (monero_wallet_obj != nullptr);
}
//-------------------------------------------------------
bool MoneroWallet::is_connected_to_daemon() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->is_connected_to_daemon();
}
//-------------------------------------------------------
bool MoneroWallet::is_synced() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->is_synced();
}
//-------------------------------------------------------
bool MoneroWallet::is_daemon_synced() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    if(!monero_wallet_obj->is_connected_to_daemon()) {
        return false;
    }
    return monero_wallet_obj->is_daemon_synced(); // will cause crash if wallet is not connected to daemon
}
//-------------------------------------------------------
//-------------------------------------------------------
bool MoneroWallet::file_exists(const std::string& filename) const {
    return monero::monero_wallet_full::wallet_exists(filename + ".keys");
}
//-------------------------------------------------------
bool MoneroWallet::is_valid_address(const std::string& address) const {
    auto wallet_network_type = get_wallet_network_type();
    return monero_utils::is_valid_address(address, static_cast<monero::monero_network_type>(wallet_network_type));
}
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------

}
