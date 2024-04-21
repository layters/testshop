#pragma once

#ifndef WALLET_HPP_NEROSHOP
#define WALLET_HPP_NEROSHOP

#define PICONERO 0.000000000001  // https://github.com/monero-project/monero/blob/master/src/cryptonote_config.h#L65 // https://web.getmonero.org/resources/moneropedia/denominations.html
#define WOWOSHI  0.00000000001   // https://git.wownero.com/wownero/wownero/src/branch/master/src/cryptonote_config.h#L68

#include <daemon/monero_daemon.h>
#include <daemon/monero_daemon_model.h>
#include <utils/gen_utils.h>
#include <utils/monero_utils.h>
#include <wallet/monero_wallet.h>
#include <wallet/monero_wallet_full.h>
#include <wallet/monero_wallet_keys.h>
#include <wallet/monero_wallet_model.h>

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
    { WalletNetworkType::Mainnet, { "18081", "18089", "34568" } },
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
    InvalidMnemonic,
    InvalidAddress,
    InvalidViewKey,
    InvalidSpendKey,
};

class Wallet : public monero_wallet_listener {
public:
    Wallet(WalletType wallet_type);
    virtual ~Wallet();

    virtual int create_random(const std::string& password, const std::string& confirm_pwd, const std::string& path);
    virtual int create_from_seed(const std::string& seed, const std::string& password, const std::string& confirm_pwd, const std::string& path);
    virtual int create_from_keys(const std::string& address, const std::string& view_key, const std::string& spend_key, const std::string& password, const std::string &confirm_pwd, const std::string& path);
    
    virtual int restore_from_seed(const std::string& seed, uint64_t restore_height = 0); // In-memory wallet
    virtual int restore_from_keys(const std::string& primary_address, const std::string& view_key, const std::string& spend_key); // In-memory wallet
    virtual int open(const std::string& path, const std::string& password); // Password-protected wallet file
    
    virtual void close(bool save = false);
    
    std::string upload(bool open = true, std::string password = "");
    
    bool verify_password(const std::string& password);
    
    // todo: create a function that connects a hardware wallet
    monero::monero_subaddress create_subaddress(unsigned int account_idx, const std::string & label = "") const; // generates a new subaddress from main account
    
    virtual void transfer(const std::string& address, double amount);
    virtual void transfer(const std::vector<std::pair<std::string, double>>& payment_addresses);
    
    std::string address_new() const; // this function has been replaced by create_subaddress() and is deprecated. Will be removed soon
    unsigned int address_book_add(const std::string& address, std::string description = "");
    void address_book_delete(unsigned int index);
    
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
    
    void wallet_info();
    
    virtual std::string sign_message(const std::string& message, monero_message_signature_type signature_type) const;
    virtual bool verify_message(const std::string& message, const std::string& signature) const;
    // setters
    void set_wallet_type(WalletType wallet_type);
    
    virtual void set_network_type(WalletNetworkType network_type);
    void set_network_type_by_string(const std::string& network_type);
    
    void set_tx_note(const std::string& txid, const std::string& tx_note);
    // getters
    WalletType get_wallet_type() const;
    virtual WalletNetworkType get_wallet_network_type() const;
    static WalletNetworkType get_network_type();
    std::string get_wallet_network_type_as_string() const;
    static std::string get_network_type_as_string();
    
    virtual std::string get_network_port() const;
    
    double get_sync_percentage() const;
    unsigned long long get_sync_height() const;
    unsigned long long get_sync_start_height() const;
    unsigned long long get_sync_end_height() const;
    std::string get_sync_message() const;
    
    virtual std::string get_primary_address() const;
    virtual std::string get_address(unsigned int index) const; // returns address at index (primary address is index 0)
    unsigned int get_address_count() const;
    
    virtual uint64_t get_balance_raw() const;
    virtual uint64_t get_balance_raw(unsigned int account_index) const;
    virtual uint64_t get_balance_raw(unsigned int account_index, unsigned int subaddress_index) const;
    virtual uint64_t get_unlocked_balance_raw() const;
    virtual uint64_t get_unlocked_balance_raw(unsigned int account_index) const;
    virtual uint64_t get_unlocked_balance_raw(unsigned int account_index, unsigned int subaddress_index) const;
    
    virtual double get_balance() const;
    virtual double get_balance(unsigned int account_index) const;
    virtual double get_balance(unsigned int account_index, unsigned int subaddress_index) const;
    virtual double get_unlocked_balance() const;
    virtual double get_unlocked_balance(unsigned int account_index) const;
    virtual double get_unlocked_balance(unsigned int account_index, unsigned int subaddress_index) const;
    
    std::vector<std::string> get_transactions() const; // "show_transfers"
    unsigned int get_transactions_count() const;
    // subaddress
    std::string get_last_subaddress() const; // returns the last subaddress to be created
    virtual std::string get_private_view_key() const; // secret view key
    virtual std::string get_public_view_key() const;
    virtual std::pair<std::string, std::string> get_view_keys() const; // secret, public
    
    virtual std::string get_private_spend_key() const; // secret spend key
    virtual std::string get_public_spend_key() const;
    virtual std::pair<std::string, std::string> get_spend_keys() const; // secret, public
    
    virtual std::string get_seed() const;
    virtual std::string get_seed_language() const;
    virtual std::vector<std::string> get_seed_languages() const;
    
    virtual std::string get_path() const;
    std::string get_type() const; // "wallet_info": Normal, HW
    virtual unsigned int get_daemon_height() const;
    virtual unsigned int get_height() const;
    virtual unsigned int get_height_by_date(int year, int month, int day) const;
    std::string get_version() const;
    // get wallet handles (monero, wownero, etc.)
    virtual void * get_handle() const;
    monero_wallet_full * get_monero_wallet() const;
    //wownero_wallet_full * get_wownero_wallet() const;
    std::vector<std::string> recent_address_list; // recently used addresses
    // boolean functions
    virtual bool is_opened() const;
    virtual bool is_connected_to_daemon() const;
    virtual bool is_synced() const;
    virtual bool is_daemon_synced() const;
    
    virtual bool file_exists(const std::string& filename) const;
    virtual bool is_valid_address(const std::string& address) const;
    static bool is_valid_monero_address(const std::string& address);
    //static bool is_valid_wownero_address(const std::string& address);
    bool is_cryptonote_based() const;
    // friends
    friend class Seller; // seller can access wallet private members
    friend class WalletController;
protected:
    WalletType wallet_type; // can switch between different wallets
    static WalletNetworkType network_type;
    std::unique_ptr<monero::monero_wallet_full> monero_wallet_obj;
    //std::unique_ptr<wownero::wownero_wallet_full> wownero_wallet_obj;
    std::unique_ptr<Process> process; // monerod process - every wallet will have its own process
    volatile double percentage; // sync progress
    mutable std::mutex wallet_data_mutex;
    volatile unsigned int height, start_height, end_height;
    /*volatile */std::string message;
    uint64_t/*unsigned long long*/ restore_height;
    std::string password_hash;
};

}
#endif // WALLET_HPP
