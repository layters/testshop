#pragma once

#ifndef WALLET_HPP_NEROSHOP
#define WALLET_HPP_NEROSHOP

#define PICONERO 0.000000000001  // the smallest unit of a monero (monero has 12 decimal places) // https://web.getmonero.org/resources/moneropedia/denominations.html
#define WOWOSHI  0.00000000001   // https://git.wownero.com/wownero/wownero/src/branch/master/src/cryptonote_config.h#L68

#include "wallets/monero.hpp"
//#include "wallets/wownero.hpp"

#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <utility> // std::pair

namespace neroshop {

class Process; // forward declaration

enum class WalletType {
    Monero = 0,
    Wownero
};

enum class WalletNetworkType : uint8_t { // refer to daemon/monero_daemon_model.h
    Mainnet = 0,
    Testnet,
    Stagenet
};

static std::map<WalletNetworkType, std::vector<std::string>> WalletNetworkPortMap {
    { WalletNetworkType::Mainnet, { "18081", "18089" } },
    { WalletNetworkType::Testnet, { "28081", "28089" } },
    { WalletNetworkType::Stagenet, { "38081", "38089" } },
};

enum class WalletError {
    Ok = 0, 
    WrongPassword, 
    PasswordsDoNotMatch, 
    AlreadyExists,
    IsOpenedByAnotherProgram,
    DoesNotExist,
    BadNetworkType,
    IsNotOpened, // monero_wallet_obj is nullptr
    BadWalletType,
};

class Wallet : public monero_wallet_listener {
public:
    Wallet();
    virtual ~Wallet();

    int create_random(const std::string& password, const std::string& confirm_pwd, const std::string& path);
    int create_from_seed(const std::string& seed, const std::string& password, const std::string& confirm_pwd, const std::string& path);
    int create_from_keys(const std::string& address, const std::string& view_key, const std::string& spend_key, const std::string& password, const std::string &confirm_pwd, const std::string& path);
    
    int restore_from_seed(const std::string& seed); // In-memory wallet
    int restore_from_keys(const std::string& primary_address, const std::string& view_key, const std::string& spend_key); // In-memory wallet
    int open(const std::string& path, const std::string& password); // Password-protected wallet file
    
    void close(bool save = false);
    
    std::string upload(bool open = true, std::string password = ""); // change to mainnet later    
    
    bool verify_password(const std::string& password);
    
    // todo: create a function that connects a hardware wallet
    monero::monero_subaddress create_subaddress(unsigned int account_idx, const std::string & label = "") const; // generates a new subaddress from main account // monero addresses start with 4 or 8
    
    void transfer(const std::string& address, double amount); // "transfer" will be used for sending refunds
    void transfer(const std::vector<std::pair<std::string, double>>& payment_addresses);
    
    std::string address_new() const; // this function has been replaced by create_subaddress() and is deprecated. Will be removed soon
    unsigned int address_book_add(const std::string& address, std::string description = ""); // "address_book add <address> <description>"
    void address_book_delete(unsigned int index); // "address_book delete 0"  
    
    static std::string generate_uri(const std::string& payment_address, double amount = 0.000000000000, const std::string& description = "", const std::string& recipient = ""); // Generates a monero uri for qr code with the amount embedded into it
    
    std::vector<monero::monero_subaddress> get_addresses_all(unsigned int account_idx);
    std::vector<monero::monero_subaddress> get_addresses_used(unsigned int account_idx);
    std::vector<monero::monero_subaddress> get_addresses_unused(unsigned int account_idx);

    std::vector<std::shared_ptr<monero_transfer>> get_transfers();

    void on_sync_progress(uint64_t height, uint64_t start_height, uint64_t end_height, double percent_done, const std::string& message);
    ////void on_new_block (uint64_t height);
    void on_balances_changed(uint64_t new_balance, uint64_t new_unlocked_balance);
    void on_output_received(const monero_output_wallet& output);
    ////void on_output_spent (const monero_output_wallet &output);
    // daemon or node-related functions
    void daemon_open(const std::string& daemon_dir, bool confirm_external_bind = false, bool restricted_rpc = true, std::string data_dir = "", unsigned int restore_height = 0);
    bool daemon_connect_local(const std::string& username = "", const std::string& password = "");
    void daemon_connect_remote(const std::string& ip, const std::string& port, const std::string& username = "", const std::string& password = "", const monero_wallet_listener* listener = nullptr);
    void daemon_close();
    //void explore_address(const std::string& address); //{Browser::open(this->addr_url + address);}// will detect address before opening explorer
    //void explore_tx(const std::string& tx_hash);
    void wallet_info();
    
    std::string sign_message(const std::string& message, monero_message_signature_type signature_type) const;
    bool verify_message(const std::string& message, const std::string& signature) const;
    // setters
    void set_wallet_type(WalletType wallet_type);
    
    void set_network_type(WalletNetworkType network_type);
    void set_network_type_by_string(const std::string& network_type);
    
    void set_tx_note(const std::string& txid, const std::string& tx_note); // "set_tx_note <txid> [free note text here]" - useful for filling address information
    // getters
    WalletType get_wallet_type() const;
    WalletNetworkType get_wallet_network_type() const;
    static WalletNetworkType get_network_type();
    std::string get_wallet_network_type_as_string() const;
    static std::string get_network_type_as_string();
    
    double get_sync_percentage() const;
    unsigned long long get_sync_height() const;
    unsigned long long get_sync_start_height() const;
    unsigned long long get_sync_end_height() const;
    std::string get_sync_message() const;
    
    std::string get_primary_address() const;
    std::string get_address(unsigned int index) const; // returns address at "index"'s string (primary address is index 0) // "address all"
    unsigned int get_address_count() const;
    
    uint64_t get_balance_raw() const;
    uint64_t get_balance_raw(unsigned int account_index) const;
    uint64_t get_balance_raw(unsigned int account_index, unsigned int subaddress_index) const;
    uint64_t get_unlocked_balance_raw() const;
    uint64_t get_unlocked_balance_raw(unsigned int account_index) const;
    uint64_t get_unlocked_balance_raw(unsigned int account_index, unsigned int subaddress_index) const;
    
    double get_balance() const;
    double get_balance(unsigned int account_index) const;
    double get_balance(unsigned int account_index, unsigned int subaddress_index) const;
    double get_unlocked_balance() const;
    double get_unlocked_balance(unsigned int account_index) const;
    double get_unlocked_balance(unsigned int account_index, unsigned int subaddress_index) const;
    
    std::vector<std::string> get_transactions() const; // "show_transfers"
    unsigned int get_transactions_count() const;
    // subaddress
    std::string get_last_subaddress() const; // returns the last subaddress to be created
    // NOTE: use address_new to automatically generate a unique subaddress for each customer to make it easier to track who and where the payments are coming from 
    std::string get_private_view_key() const; // secret view key
    std::string get_public_view_key() const;
    std::pair<std::string, std::string> get_view_keys() const; // secret, public // "viewkey"
    std::string get_private_spend_key() const; // secret spend key
    std::string get_public_spend_key() const;
    std::pair<std::string, std::string> get_spend_keys() const; // secret, public // "spendkey"
    std::string get_seed() const; // "seed"
    std::string get_path() const;
    //std::string get_description() const;
    std::string get_type() const; // "wallet_info": Normal, HW
    unsigned int get_daemon_height() const;
    unsigned int get_height() const;
    unsigned int get_height_by_date(int year, int month, int day) const;
    std::string get_status() const; // "status" - Check current status of wallet.
    std::string get_version() const; // "version" - Check software version.
    // get wallet handles (monero, wownero, etc.)
    monero_wallet_full * get_monero_wallet() const;
    //wownero_wallet_full * get_wownero_wallet() const;
    std::vector<std::string> recent_address_list; // recently used addresses
    // boolean functions
    bool is_opened() const;
    bool is_connected_to_daemon() const;
    bool is_synced() const;
    bool is_daemon_synced() const;
    
    bool file_exists(const std::string& filename) const;
    bool is_valid_address(const std::string& address) const;
    bool is_cryptonote_based() const;
    // friends
    friend class Seller; // seller can access wallet private members
    friend class WalletController;
private:
    void set_daemon(); // "set_daemon <host>[:<port>] [trusted|untrusted|this-is-probably-a-spy-node]" - connects to a daemon
    void refresh(); // "refresh" - Synchronize wallet with the Monero network.
    void config(); // loads a config file (wallet_config.txt) with wallet settings
    // callbacks
    void load_from_config(std::string/*const std::string&*/ password = "supersecretpassword123");
private:
    WalletType wallet_type; // can switch between different wallets
    static WalletNetworkType network_type;
    std::unique_ptr<monero::monero_wallet_full> monero_wallet_obj; // monero wallet
    //std::unique_ptr<wownero::wownero_wallet_full> wownero_wallet_obj;
    std::unique_ptr<Process> process; // monerod process // every wallet will have its own process
    volatile double percentage; // sync progress
    mutable std::mutex wallet_data_mutex;
    volatile unsigned int height, start_height, end_height;
    /*volatile */std::string message;
    uint64_t/*unsigned long long*/ restore_height;
    std::string password_hash; // pw hash that is only stored in memory
};

}
#endif // WALLET_HPP
