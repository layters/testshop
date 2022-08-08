#pragma once

#ifndef WALLET_HPP_NEROSHOP
#define WALLET_HPP_NEROSHOP

#define EXPLORER_XMRCHAIN             "https://xmrchain.net/search?value="
#define EXPLORER_XMRCHAIN_MAINNET     EXPLORER_XMRCHAIN
#define EXPLORER_XMRCHAIN_STAGENET    "https://stagenet.xmrchain.net/search?value="
#define EXPLORER_XMRCHAIN_TESTNET     "https://testnet.xmrchain.net/search?value="
#define EXPLORER_XMRCHAIN_TX          "https://xmrchain.net/tx/"
#define EXPLORER_XMRCHAIN_MAINNET_TX  EXPLORER_XMRCHAIN_TX
#define EXPLORER_XMRCHAIN_STAGENET_TX "https://stagenet.xmrchain.net/tx/"
#define EXPLORER_XMRCHAIN_TESTNET_TX  "https://testnet.xmrchain.net/tx/"

#define NEROSHOP_DEFAULT_WALLET_PATH "/home/" + neroshop::device::get_user() + "/neroshop"//"/home/" + neroshop::device::get_user() + "/Monero/wallets"

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
#include <vector>
#include <utility> // std::pair
#include <cmath>

#include "process.hpp" // for monerod daemon process
#include "message.hpp"

namespace neroshop {

class Wallet : public monero_wallet_listener {
public:
    Wallet();
    ~Wallet();

    void create_random(const std::string& password, const std::string& confirm_pwd, const std::string& path);
    void create_from_mnemonic(const std::string& mnemonic, const std::string& password, const std::string& confirm_pwd, const std::string& path);
    void create_from_keys(const std::string& address, const std::string& view_key, const std::string& spend_key, const std::string& password, const std::string &confirm_pwd, const std::string& path);
    
    void restore_from_mnemonic(const std::string& mnemonic); // In-memory wallet
    void restore_from_keys(const std::string& primary_address, const std::string& view_key, const std::string& spend_key); // In-memory wallet
    void open(const std::string& path, const std::string& password); // Password-protected wallet file
    
    void close(bool save = false);
    
    std::string upload(bool open = true, std::string password = ""); // change to mainnet later    
    
    // todo: create a function that connects a hardware wallet
    
    void transfer(const std::string& address, double amount); // "transfer" will be used for sending refunds
    void sweep_all(const std::string& address); // sends entire balance, including dust to an address // "sweep_all <address>"
    std::string address_new() const; // generates a new subaddress from main account // "address new" // monero addresses start with 4 or 8
    unsigned int address_book_add(const std::string& address, std::string description = ""); // "address_book add <address> <description>"
    void address_book_delete(unsigned int index); // "address_book delete 0"  
    std::vector<std::string> address_all(); // show all addresses
    std::vector<std::string> address_used(); // show all used addresses
    std::vector<std::string> address_unused(); // show all unused addresses

    void on_sync_progress(uint64_t height, uint64_t start_height, uint64_t end_height, double percent_done, const std::string& message);
    ////void on_new_block (uint64_t height);
    void on_balances_changed(uint64_t new_balance, uint64_t new_unlocked_balance);
    void on_output_received(const monero_output_wallet& output);
    ////void on_output_spent (const monero_output_wallet &output);
    // daemon or node-related functions
    void daemon_open(const std::string& ip, const std::string& port, bool confirm_external_bind = false, bool restricted_rpc = true, bool remote = false, std::string data_dir = std::string("/home/") + System::get_user() + std::string("/.bitmonero")/*""*/, std::string network_type = "stagenet", unsigned int restore_height = 0);
    bool daemon_connect(const std::string& ip, const std::string& port);
    void daemon_close();
    //void explore_address(const std::string& address); //{Browser::open(this->addr_url + address);}// will detect address before opening explorer
    //void explore_tx(const std::string& tx_hash);
    void wallet_info();
    // setters
    void set_tx_note(const std::string& txid, const std::string& tx_note); // "set_tx_note <txid> [free note text here]" - useful for filling address information
    // getters
    std::string get_primary_address() const; // returns primary address string // "address"
    std::string get_address(unsigned int index) const; // returns address at "index"'s string (primary address is index 0) // "address all"
    unsigned int get_address_count() const; // address_list.size();
    double get_balance_raw(unsigned int account_index = 0, unsigned int subaddress_index = 0) const; // "balance"
    double get_unlocked_balance_raw(unsigned int account_index = 0, unsigned int subaddress_index = 0) const; // "balance"
    double get_balance(unsigned int account_index = 0, unsigned int subaddress_index = 0) const;
    double get_unlocked_balance(unsigned int account_index = 0, unsigned int subaddress_index = 0) const;
    std::vector<std::string> get_transactions() const; // "show_transfers"
    unsigned int get_transactions_count() const;
    // subaddress
    std::string get_last_subaddress() const; // returns the last subaddress to be created
    // proof (proving the transaction was submitted) - https://web.getmonero.org/resources/user-guides/prove-payment.html
    ////std::string get_tx_note(const std::string& txid) const; // "get_tx_note <txid>" - useful for retrieving address information
    // get_spend_proof <txid> [<message>]
    // get_tx_proof <txid> <address> [<message>]
    // get_reserve_proof (all|<amount>) [<message>]
    // check_reserve_proof <address> <signature_file> [<message>]
    // check_spend_proof <txid> <signature_file> [<message>]
    // check_tx_proof <txid> <address> <signature_file> [<message>]
    // NOTE: use address_new to automatically generate a unique subaddress for each customer to make it easier to track who and where the payments are coming from 
    std::string get_private_view_key() const; // secret view key
    std::string get_public_view_key() const;
    std::pair<std::string, std::string> get_view_keys() const; // secret, public // "viewkey"
    std::string get_private_spend_key() const; // secret spend key
    std::string get_public_spend_key() const;
    std::pair<std::string, std::string> get_spend_keys() const; // secret, public // "spendkey"
    std::string get_mnemonic() const; // "seed"
    //-Image * get_qr_code() const; // returns address qrcode // "show_qr_code"
    //-Image * get_qr_code(unsigned int address_index) const; // returns the qrcode of the address at "index"
    std::string get_path() const; // "wallet_info"
    std::string get_description() const; // "wallet_info"    
    std::string get_type() const; // "wallet_info": Normal, HW
    unsigned int get_daemon_height() const;
    unsigned int get_height() const;
    unsigned int get_height_by_date(int year, int month, int day) const;
    monero::monero_network_type get_network_type() const; // "wallet_info":  Mainnet, Testnet, Stagenet
    std::string get_network_type_str() const; // "wallet_info":  Mainnet, Testnet, Stagenet
    std::string get_status() const; // "status" - Check current status of wallet.
    std::string get_version() const; // "version" - Check software version.
    // get wallet handles (monero, wownero, etc.)
    monero_wallet_full * get_monero_wallet() const;
    std::vector<std::string> recent_address_list; // recently used addresses
    // friends
    friend class Seller; // seller can access wallet private members
private:
    void set_daemon(); // "set_daemon <host>[:<port>] [trusted|untrusted|this-is-probably-a-spy-node]" - connects to a daemon
    void refresh(); // "refresh" - Synchronize wallet with the Monero network.
    void config(); // loads a config file (wallet_config.txt) with wallet settings
    // callbacks
    void load_from_config(std::string/*const std::string&*/ password = "supersecretpassword123");
private:
    // monero-related
    std::unique_ptr<monero::monero_wallet_full> monero_wallet_obj; // monero wallet
    monero::monero_network_type network_type; // default will be mainnet when this application is released
    // dokun-ui
    std::unique_ptr<Process> process; // monerod process // every wallet will have its own process
    //std::unique_ptr<Progressbar> sync_bar;
};

}
#endif // WALLET_HPP
