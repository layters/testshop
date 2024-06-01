#pragma once

#ifndef MONERO_HPP_NEROSHOP
#define MONERO_HPP_NEROSHOP

#include "../wallet.hpp"

namespace neroshop {

class MoneroWallet : public Wallet/*, public monero_wallet_listener*/ {
public:
    MoneroWallet();
    ~MoneroWallet();
    
    int create_random(const std::string& password, const std::string& confirm_pwd, const std::string& path) override;
    int create_from_seed(const std::string& seed, const std::string& password, const std::string& confirm_pwd, const std::string& path) override;
    int create_from_keys(const std::string& address, const std::string& view_key, const std::string& spend_key, const std::string& password, const std::string &confirm_pwd, const std::string& path) override;
    
    int restore_from_seed(const std::string& seed, uint64_t restore_height = 0) override;
    int restore_from_keys(const std::string& primary_address, const std::string& view_key, const std::string& spend_key) override;
    
    int open(const std::string& path, const std::string& password) override;
    void close(bool save = false) override;
    
    bool change_password(const std::string& old_password, const std::string& new_password) override;
    
    std::string sign_message(const std::string& message, monero_message_signature_type signature_type) const override;
    bool verify_message(const std::string& message, const std::string& signature) const override;
    
    void transfer(const std::string& address, double amount) override;
    void transfer(const std::vector<std::pair<std::string, double>>& payment_addresses) override;
    void transfer(const std::string& uri) override;
    
    std::string make_uri(const std::string& payment_address, double amount = 0.000000000000, const std::string& description = "", const std::string& recipient = "") const override;
    
    void set_network_type(WalletNetworkType network_type) override;
    
    WalletNetworkType get_wallet_network_type() const override;
    std::string get_network_port() const override;
    
    std::string get_primary_address() const override;
    std::string get_address(unsigned int index) const override;
    
    uint64_t get_balance_raw() const override;
    uint64_t get_balance_raw(unsigned int account_index) const override;
    uint64_t get_balance_raw(unsigned int account_index, unsigned int subaddress_index) const override;
    uint64_t get_unlocked_balance_raw() const override;
    uint64_t get_unlocked_balance_raw(unsigned int account_index) const override;
    uint64_t get_unlocked_balance_raw(unsigned int account_index, unsigned int subaddress_index) const override;
    
    double get_balance() const override;
    double get_balance(unsigned int account_index) const override;
    double get_balance(unsigned int account_index, unsigned int subaddress_index) const override;
    double get_unlocked_balance() const override;
    double get_unlocked_balance(unsigned int account_index) const override;
    double get_unlocked_balance(unsigned int account_index, unsigned int subaddress_index) const override;
    
    std::string get_private_view_key() const override;
    std::string get_public_view_key() const override;
    std::pair<std::string, std::string> get_view_keys() const override;
    
    std::string get_private_spend_key() const override;
    std::string get_public_spend_key() const override;
    std::pair<std::string, std::string> get_spend_keys() const override;
    
    std::string get_seed() const override;
    std::string get_seed_language() const override;
    std::vector<std::string> get_seed_languages() const override;
    
    std::string get_path() const override;
    
    unsigned int get_daemon_height() const override;
    unsigned int get_height() const override;
    unsigned int get_height_by_date(int year, int month, int day) const override;
    
    void * get_handle() const override;
    
    bool is_opened() const override;
    bool is_connected_to_daemon() const override;
    bool is_synced() const override;
    bool is_daemon_synced() const override;
    
    bool file_exists(const std::string& filename) const override;
    bool is_valid_address(const std::string& address) const override;
};

}
#endif // MONERO_HPP_NEROSHOP
