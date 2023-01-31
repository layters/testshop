#include "wallet.hpp"
////////////////////
////////////////////
// constructors and destructors
////////////////////
neroshop::Wallet::Wallet() : process(nullptr), percentage(0.0), monero_wallet_obj(nullptr), network_type(monero_network_type::STAGENET)
{}
////////////////////
neroshop::Wallet::~Wallet() 
{
    if(monero_wallet_obj.get()) {
        // remove listener
        monero_wallet_obj->remove_listener (*this);//int listener_count = monero_wallet_obj->get_listeners().size();if(listener_count > 0) { std::cout << NEROSHOP_TAG "still need to delete listeners (" << listener_count << ")" << std::endl; }
        // close monero wallet
        close(false);
        // reset (delete) monero wallet
        monero_wallet_obj.reset(); // will call monero_wallet_full destructor
        if(!monero_wallet_obj.get()) std::cout << "monero wallet deleted\n"; // just to confirm that the monero wallet has been set to nullptr after deletion
    }
    // destroy process
    if(process.get()) {
        process.reset(); // this will call the process destructor which should auto kill the process
        if(!process.get()) std::cout << "wallet process deleted\n";
    }
#ifdef NEROSHOP_DEBUG    
    std::cout << "wallet deleted\n";
#endif    
}
////////////////////
////////////////////
// 	Thought: account creation date could be used as the wallet's restore height
////////////////////
// Reminder: path is the path and name of the wallet without the .keys extension
neroshop::wallet_error neroshop::Wallet::create_random(const std::string& password, const std::string& confirm_pwd, const std::string& path) {
    if(confirm_pwd != password) {
        neroshop::print("Wallet passwords do not match", 1);
        return neroshop::wallet_error::PASSWORDS_NO_MATCH;
    }
    
    if(monero::monero_wallet_full::wallet_exists(path + ".keys")) {//if(std::filesystem::is_regular_file(path + ".keys")) {
        neroshop::print("Wallet file with the same name already exists", 1);
        return neroshop::wallet_error::WALLET_ALREADY_EXISTS;
    }
    // This is deprecated :(
    ////monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet_random (path, password, network_type/*, const monero_rpc_connection &daemon_connection=monero_rpc_connection(), const std::string &language="English", std::unique_ptr< epee::net_utils::http::http_client_factory > http_client_factory=nullptr*/));
    
    monero::monero_wallet_config wallet_config_obj;
    wallet_config_obj.m_path = path;
    wallet_config_obj.m_password = password;
    wallet_config_obj.m_network_type = this->network_type;
    
    monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet (wallet_config_obj, nullptr));
    if(monero_wallet_obj.get()) std::cout << "\033[1;35;49m" << "created wallet \"" << path << ".keys\"" << "\033[0m" << std::endl;
    return neroshop::wallet_error::WALLET_SUCCESS;
}
////////////////////
bool neroshop::Wallet::create_from_mnemonic(const std::string& mnemonic, const std::string& password, const std::string& confirm_pwd, const std::string& path) {
    if(confirm_pwd != password) {
        neroshop::print("Wallet passwords do not match", 1);
        return false;
    }    

    if(monero::monero_wallet_full::wallet_exists(path + ".keys")) {
        neroshop::print("Wallet file with the same name already exists", 1);
        return false;
    }
    // This is deprecated :(    
    ////monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet_from_mnemonic (path, password, network_type, mnemonic));
    
    monero::monero_wallet_config wallet_config_obj;
    wallet_config_obj.m_path = path;
    wallet_config_obj.m_password = password;
    wallet_config_obj.m_network_type = this->network_type;
    wallet_config_obj.m_mnemonic = mnemonic;
    
    monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet (wallet_config_obj, nullptr));
    if(!monero_wallet_obj.get()) return false;
    std::cout << "\033[1;35;49m" << "created wallet \"" << path << ".keys\" (from mnemonic)" << "\033[0m" << std::endl;
    return true;
}
////////////////////
// To restore a view-only wallet, leave the spend key blank
bool neroshop::Wallet::create_from_keys(const std::string& primary_address, const std::string& view_key, const std::string& spend_key, const std::string& password, const std::string &confirm_pwd, const std::string& path) {
    if(confirm_pwd != password) {
        neroshop::print("Wallet passwords do not match", 1);
        return false;
    }  
    
    if(monero::monero_wallet_full::wallet_exists(path + ".keys")) {
        neroshop::print("Wallet file with the same name already exists", 1);
        return false;
    }
    // This is deprecated :(    
    ////monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet_from_keys(path, password, network_type, primary_address, view_key, spend_key));

    monero::monero_wallet_config wallet_config_obj;
    wallet_config_obj.m_path = path;
    wallet_config_obj.m_password = password;
    wallet_config_obj.m_network_type = this->network_type;
    wallet_config_obj.m_primary_address = primary_address;
    wallet_config_obj.m_private_view_key = view_key;
    wallet_config_obj.m_private_spend_key = spend_key;
    
    monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet (wallet_config_obj, nullptr));
    if(!monero_wallet_obj.get()) return false;
    std::cout << "\033[1;35;49m" << "created wallet \"" << path << ".keys\" (from keys)" << "\033[0m" << std::endl;
    return true;
}
////////////////////
bool neroshop::Wallet::restore_from_mnemonic(const std::string& mnemonic) 
{
    // This is deprecated :(
    //monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet_from_mnemonic("", "", network_type, mnemonic)); // set path to "" for an in-memory wallet
    
    monero::monero_wallet_config wallet_config_obj;
    wallet_config_obj.m_path = "";
    wallet_config_obj.m_password = "";
    wallet_config_obj.m_network_type = this->network_type;
    wallet_config_obj.m_mnemonic = mnemonic;
    wallet_config_obj.m_restore_height = 0;
    
    monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet (wallet_config_obj, nullptr));
    if(!monero_wallet_obj.get()) return false;
    std::cout << "\033[1;35;49m" << "restored in-memory wallet (from mnemonic)" << "\033[0m" << std::endl;    
    return true;
}
////////////////////
bool neroshop::Wallet::restore_from_keys(const std::string& primary_address, const std::string& view_key, const std::string& spend_key)
{
    // Check validity of primary address
    if(!monero_utils::is_valid_address(primary_address, this->network_type)) {
        neroshop::print("Invalid Monero address", 1);
        return false;
    }
    // This is deprecated :(
    //monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet_from_keys ("", "", network_type, primary_address, view_key, spend_key)); // set path to "" for an in-memory wallet
    
    monero::monero_wallet_config wallet_config_obj;
    wallet_config_obj.m_path = "";
    wallet_config_obj.m_password = "";
    wallet_config_obj.m_network_type = this->network_type;
    wallet_config_obj.m_primary_address = primary_address;
    wallet_config_obj.m_private_view_key = view_key;
    wallet_config_obj.m_private_spend_key = spend_key;
    
    monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero_wallet_full::create_wallet (wallet_config_obj, nullptr));    
    if(!monero_wallet_obj.get()) return false;
    std::cout << "\033[1;35;49m" << "restored in-memory wallet (from keys)" << "\033[0m" << std::endl;
    return true;
}
////////////////////
bool neroshop::Wallet::open(const std::string& path, const std::string& password) { 
    if(!monero::monero_wallet_full::wallet_exists(path)) { 
        return false; 
    }

    try {
        monero_wallet_obj = std::unique_ptr<monero_wallet_full>(monero::monero_wallet_full::open_wallet(path, password, this->network_type));
    } catch (const std::exception& e) {
        neroshop::print(e.what(), 1);//tools::error::invalid_password
        return false;
    }
    if(!monero_wallet_obj.get()) { 
        return false;
    }
    std::cout << "\033[1;35;49m" << "opened wallet \"" << path << "\"\033[0m" << std::endl;
    return true;
}
////////////////////
void neroshop::Wallet::close(bool save) 
{
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    monero_wallet_obj->close(save);
}
////////////////////
std::string neroshop::Wallet::upload(bool open, std::string password) { // opens the wallet file
    // open file dialog to retrieve walletfile path
    // reminder: use this function for an upload button instead
    //           then call neroshop::Wallet::open when user presses a "submit" button.
    char file[1024];
#ifdef __gnu_linux__
    FILE *f = popen("zenity --file-selection --title \"Select Wallet\"", "r");//--filename "/home/${USER}/"
    fgets(file, 1024, f);
    // consider https://github.com/AndrewBelt/osdialog/blob/master/osdialog_zenity.c
#endif
    std::string filename(file); // "wallet.keys" file
    filename = filename.substr(0, filename.find(".")); // remove ".keys" extension
    if(!monero::monero_wallet_full::wallet_exists(filename + ".keys")) { neroshop::print("wallet not found", 1); return ""; } // check if wallet file is valid (or exists)
    if(open == true) neroshop::Wallet::open(filename, password);// will apply ".keys" ext to the wallet file
    return std::string(filename + ".keys");
}
////////////////////
// refer to https://moneroecosystem.org/monero-cpp/structmonero_1_1monero__tx__config.html
////////////////////
void neroshop::Wallet::transfer(const std::string& address, double amount) {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    if(!monero_wallet_obj.get()->is_synced()) throw std::runtime_error("wallet is not synced with a daemon");
    // Convert monero to piconero
    double piconero = 0.000000000001;
    uint64_t monero_to_piconero = amount / piconero; //std::cout << neroshop::string::precision(amount, 12) << " xmr to piconero: " << monero_to_piconero << "\n";
    // TODO: for the 2-of-3 escrow system, take 0.5% of order total in piconeros
    // Check if balance is sufficient
    std::cout << "Wallet balance (spendable): " << monero_wallet_obj->get_unlocked_balance() << " (picos)\n";
    std::cout << "Amount to send: " << monero_to_piconero << " (picos)\n";
    if(monero_to_piconero == 0) return;
    if(monero_wallet_obj->get_unlocked_balance() < monero_to_piconero) {
        neroshop::print("Wallet balance is insufficient", 1); return;
    }
    if(monero_wallet_obj->get_unlocked_balance() == monero_to_piconero) {
        neroshop::print("Not enough unlocked balance for tx fees. Sweep balance?");// Sweep balance?
    }
    // Check if address is valid
    if(!monero_utils::is_valid_address(address, monero_wallet_obj->get_network_type())) {
        neroshop::print("Monero address is invalid", 1); return;
    }
    //Configures a transaction to send, sweep, or create a payment URI.
    // send funds from this wallet to the specified address
    monero_tx_config config;
    config.m_account_index = 0; // withdraw funds from account at index 0
    config.m_address = address; // address that will be receiving the funds
    config.m_amount = monero_to_piconero;
    config.m_relay = true;
    std::shared_ptr<monero_tx_wallet> sent_tx = monero_wallet_obj->create_tx(config);
    bool in_pool = sent_tx->m_in_tx_pool.get();  // true
        
    //uint64_t fee = sent_tx->m_fee.get(); // "Are you sure you want to send ...?"
    //monero_wallet_obj->relay_tx(*sent_tx); // recipient receives notification within 5 seconds    
    // prove that you've sent the payment using "get_tx_proof"
}
////////////////////
std::string neroshop::Wallet::sign_message(const std::string& message, monero_message_signature_type signature_type) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->sign_message(message, signature_type, 0, 0);
}
////////////////////
bool neroshop::Wallet::verify_message(const std::string& message, const std::string& signature) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->verify_message(message, monero_wallet_obj.get()->get_primary_address(), signature).m_is_good;
}
////////////////////
////////////////////
monero::monero_subaddress neroshop::Wallet::create_subaddress(unsigned int account_idx, const std::string & label) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    monero_subaddress subaddress = monero_wallet_obj->create_subaddress(account_idx, label);
    // we have to make sure that each newly generated subaddress is unique (meaning it has never been used)
    if(subaddress.m_is_used.get()) {
        throw std::runtime_error("subaddress " + subaddress.m_address.get() + " was previously used. Is your wallet node properly synced?");////neroshop::print("subaddress " + subaddress.m_address.get() + " was previously used");
    }    
    return subaddress;
}
////////////////////
////////////////////
void neroshop::Wallet::sweep_all(const std::string& address) {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    //std::vector< std::shared_ptr< monero_tx_wallet > > monero::monero_wallet_full::sweep_dust 	( 	bool  	relay = false	) 	
} // sends entire balance, including dust to an address // "sweep_all <address>"
////////////////////
////////////////////
// NOTE: It is IMPOSSIBLE to change the network type of a pre-existing monero wallet, but it can be set before its creation
void neroshop::Wallet::set_network_type(monero::monero_network_type network_type) {
    if(get_network_type() == network_type) return;
    if(monero_wallet_obj.get()) throw std::runtime_error("Cannot change the network type of " + get_network_type_string() + " wallet");
    this->network_type = network_type;
}
////////////////////
void neroshop::Wallet::set_network_type_by_string(const std::string& network_type) {
    if(network_type == "mainnet") set_network_type(monero_network_type::MAINNET);
    if(network_type == "testnet") set_network_type(monero_network_type::TESTNET);
    if(network_type == "stagenet") set_network_type(monero_network_type::STAGENET);
    std::cout << "Monero network type has been set to: " << network_type << std::endl;
}
////////////////////
////////////////////
std::string neroshop::Wallet::address_new() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    // Create a subaddress within an account (account_0 to be specific).
    monero_subaddress subaddress = monero_wallet_obj->create_subaddress(0);//monero_subaddress monero::monero_wallet_full::create_subaddress	(	uint32_t 	account_idx,const std::string & 	label = "" )
#ifdef NEROSHOP_DEBUG
    std::cout << "address_new: " << subaddress.m_address.get() << " (account_idx: " << subaddress.m_account_index.get() << ", subaddress_idx: " << subaddress.m_index.get() << ")" << std::endl;
#endif
    //if subaddress has already been used
    if(subaddress.m_is_used.get()) { std::cout << "subaddress " << subaddress.m_address.get() << " already in use!" << std::endl; return ""; }
    return subaddress.m_address.get();// return the newly created subaddress
    // store new subaddress//subaddress_list.push_back(subaddress.m_address.get());
} // generates a new subaddress from main account // "address new"
////////////////////
unsigned int neroshop::Wallet::address_book_add(const std::string& address, std::string description) {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->add_address_book_entry(address, description);//unsigned int index = monero::monero_wallet_full::add_address_book_entry	(	address, const std::string & 	description ); // adds an address book entry and returns the index of the added entry
}
////////////////////
void neroshop::Wallet::address_book_delete(unsigned int index) {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    monero_wallet_obj->delete_address_book_entry(index);//void monero::monero_wallet_full::delete_address_book_entry	(	uint64_t 	index	);
}
////////////////////
//void  neroshop::Wallet::explore(const std::string& address) {} // will detect address before opening explorer
////////////////////
std::vector<monero::monero_subaddress> neroshop::Wallet::get_addresses_all(unsigned int account_idx) {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    if(!monero_wallet_obj.get()->is_synced()) throw std::runtime_error("wallet is not synced with a daemon");
    std::vector<monero::monero_subaddress> addresses = {};
    std::vector<unsigned int> subaddress_indices = {};
    for(int i = 0; i < monero_wallet_obj->get_account(account_idx, true).m_subaddresses.size(); i++) {
        addresses = monero_wallet_obj->get_subaddresses(account_idx, subaddress_indices); // retrieve subaddress from account at 'account_idx'
        std::cout << addresses[i].m_index.get() << " " << addresses[i].m_address.get() << (addresses[i].m_is_used.get() ? " (used)" : "") << std::endl;
    }
    return addresses;
}

std::vector<monero::monero_subaddress> neroshop::Wallet::get_addresses_used(unsigned int account_idx) {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    if(!monero_wallet_obj.get()->is_synced()) throw std::runtime_error("wallet is not synced with a daemon");
    std::vector<monero::monero_subaddress> addresses = {};
    std::vector<unsigned int> subaddress_indices = {};
    for(int i = 0; i < monero_wallet_obj->get_account(account_idx, true).m_subaddresses.size(); i++) {
        addresses = monero_wallet_obj->get_subaddresses(account_idx, subaddress_indices); // retrieve subaddress from account at 'account_idx'
        if(addresses[i].m_is_used.get()) {
            std::cout << addresses[i].m_index.get() << " " << addresses[i].m_address.get() << std::endl;
        }
    }
    return addresses;
}

std::vector<monero::monero_subaddress> neroshop::Wallet::get_addresses_unused(unsigned int account_idx) {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    if(!monero_wallet_obj.get()->is_synced()) throw std::runtime_error("wallet is not synced with a daemon");
    std::vector<monero::monero_subaddress> addresses = {};
    std::vector<unsigned int> subaddress_indices = {};
    for(int i = 0; i < monero_wallet_obj->get_account(account_idx, true).m_subaddresses.size(); i++) {
        std::vector<monero::monero_subaddress> unused_addresses = monero_wallet_obj->get_subaddresses(account_idx, subaddress_indices); // retrieve subaddress from account at 'account_idx'
        if(!unused_addresses[i].m_is_used.get()) {
            std::cout << ((std::find(recent_address_list.begin(), recent_address_list.end(), unused_addresses[i].m_address.get()) != recent_address_list.end()) ? "\033[91m" : "\033[0m") << unused_addresses[i].m_index.get() << " " << unused_addresses[i].m_address.get() << "\033[0m" << std::endl;// if subaddress is found in recent_address_list, mark it red
            // skip any recently used subaddress (to ensure the uniqueness of a subaddress)
            if(std::find(recent_address_list.begin(), recent_address_list.end(), unused_addresses[i].m_address.get()) != recent_address_list.end()) 
                continue;
            addresses.push_back(unused_addresses[i]); // store unused addresses in vector
        }
    }
    return addresses;
}

////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
// override
////////////////////
// I get the error: "../../src/xcb_io.c:641: _XReply: Assertion `!xcb_xlib_threads_sequence_lost' failed." when using this function
// or maybe its just some random threading error
void neroshop::Wallet::on_sync_progress(uint64_t height, uint64_t start_height, uint64_t end_height, double percent_done, const std::string& message) {
	    // all of this takes place in a separate thread
	    // if monero_wallet is already synced, skip this function (this function keeps spamming throughout the entire app session -.-)
	    if(monero_wallet_obj.get()->is_synced()) return;
        std::cout << "Node Sync Thread ID: " << std::this_thread::get_id() << std::endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::lock_guard<std::mutex> lock(wallet_data_mutex);
	    auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now); // current time
	    std::stringstream ss;
	    ss << std::put_time(std::localtime(&in_time_t), "[%Y-%m-%d  %l:%M:%S %p]");
	    std::string date = ss.str();
        std::cout << "\033[0;35;49m" << date << " \033[1;33;49m" << "**********************************************************************" << "\033[0m" << std::endl;
        std::cout << "\033[0;35;49m" << date << " \033[1;33;49m" << "Synced " << height << "/" << end_height;//<< "\033[0m" << std::endl;
        unsigned int blocks_to_sync = end_height - height;
        std::cout << "\033[1;33;49m" << " [Your node is " << blocks_to_sync << " block(s) behind]" << "\033[0m" << std::endl;//" blocks (" << blocks_to_sync / 60? << " minutes) behind]" << "\033[0m" << std::endl; // 1 block = 1 minute
        std::cout << "\033[0;35;49m" << date << " \033[1;33;49m" << message << ": " << (percent_done * 100) << "% (actual: " << percent_done << ")\033[0m" << std::endl;
        if((percent_done * 100) == 100) std::cout << "\033[0;35;49m" << date << " \033[1;32;49m" << "SYNCHRONIZATION DONE" << std::endl;
        std::cout << "\033[0;35;49m" << date << std::endl;
        std::cout << "\033[0;35;49m" << date << " \033[1;33;49m" << "**********************************************************************" << "\033[0m" << std::endl;
        
        this->percentage = percent_done;
        this->height = height;
        this->start_height = start_height;
        this->end_height = end_height;
        this->message = message;
        
        if(percent_done == 1) this->height = end_height;
}
////////////////////
void neroshop::Wallet::on_output_received(const monero_output_wallet& output) {
        // if wallet is not fully synced with a daemon, skip this function
        if(!monero_wallet_obj.get()) return;
        if(!monero_wallet_obj.get()->is_synced()) return;
        uint64_t amount = output.m_amount.get();
        std::string tx_hash = output.m_tx->m_hash.get();
        bool is_confirmed = output.m_tx->m_is_confirmed.get(); // unlocked_balance (can be spent)
        bool is_locked = std::dynamic_pointer_cast<monero_tx_wallet>(output.m_tx)->m_is_locked.get(); // balance (locked - still processing)
        int account_index = output.m_account_index.get(); // should always be 0 (default)
        int subaddress_index = output.m_subaddress_index.get();
        // get balance (actual)
        double piconero = 0.000000000001;
        double balance = (amount * piconero);
        // get the subaddress
        std::string subaddress = get_address(subaddress_index);
        // you've received some xmr but it may be unspendable for the meantime
        if(is_locked) {
            std::cout << "\033[1;92;49m" << "TX incoming (amount: " << std::fixed << std::setprecision(12) << balance << std::fixed << std::setprecision(2) << " xmr, txid: " << tx_hash << ", account_idx: " << account_index << ", subaddress_idx: " << subaddress_index << ")" << "\033[0m" << std::endl;
            // store subaddress in recent_address_list (if not yet added)
            if(std::find(recent_address_list.begin(), recent_address_list.end(), subaddress) == recent_address_list.end())
                recent_address_list.push_back(subaddress);//std::cout << "recently used addresses (count): " << recent_address_list.size() << std::endl;
        }
        // your xmr can now be spent freely :)
        // at this point, any recently used subaddress will be removed from vector returned by Wallet::addresses_unused() (this is the final confirmation) - sometimes this message shows twice
        if(is_confirmed) { 
            std::cout << "\033[1;32;49m" << "You have received " << std::fixed << std::setprecision(12) << balance << std::fixed << std::setprecision(2) << " xmr " << "(txid: " << tx_hash << ", account_idx: " << account_index << ", subaddress_idx: " << subaddress_index << ")" << "\033[0m" << std::endl;
            // set message box text then show message box
            // box text0 (label)
            /*neroshop::Message::get_second()->set_text(String::to_string_with_precision(balance, 12) + " xmr was deposited into your account ", 0, 107, 61);//56, 117, 11);//if(get_monero_wallet() != nullptr) //neroshop::Message::get_second()->set_text(std::string("You have received " + String::to_string_with_precision(balance, 12) + " xmr"), 56, 117, 11);//34, 139, 34);//neroshop::Message message(std::string("output received: " + std::to_string(balance) + " xmr"), 34, 139, 34);//(address: " + String::get_first_n_characters(subaddress, 4) + ".." + String::get_last_n_characters(subaddress, 4) + ")"
            neroshop::Message::get_second()->get_label(0)->set_alignment("none");
            neroshop::Message::get_second()->get_label(0)->set_relative_position((neroshop::Message::get_second()->get_width() / 2) - (neroshop::Message::get_second()->get_label(0)->get_width() / 2), ((neroshop::Message::get_second()->get_height() - neroshop::Message::get_second()->get_label(0)->get_height()) / 2) - 20);
            std::cout << neroshop::Message::get_second()->get_label(0)->get_string() << " (width: " << (neroshop::Message::get_second()->get_label(0)->get_string().length() * 10) << ")" << std::endl;
            // box text1
            int text_gap = 10;//5; // space between text0 and text1 (vertically)
            neroshop::Message::get_second()->set_text(String::get_first_n_characters(subaddress, 15) + ".." + String::get_last_n_characters(subaddress, 15) + " (idx: " + std::to_string(subaddress_index) + ")", 1);
            neroshop::Message::get_second()->get_label(1)->set_alignment("none");//neroshop::Message::get_second()->get_label(0)->get_relative_x()
            neroshop::Message::get_second()->get_label(1)->set_relative_position(((neroshop::Message::get_second()->get_width() / 2) - (neroshop::Message::get_second()->get_label(1)->get_string().length()*10 / 2)) - (10 * std::to_string(subaddress_index).length()), neroshop::Message::get_second()->get_label(0)->get_relative_y() + neroshop::Message::get_second()->get_label(0)->get_height() + text_gap); // 1          
            // cancel button
            neroshop::Message::get_second()->get_button(0)->set_text("Close");
            neroshop::Message::get_second()->get_button(0)->set_color(214, 31, 31, 1.0);                    
            neroshop::Message::get_second()->get_button(0)->set_relative_position((neroshop::Message::get_second()->get_width() / 2) - (neroshop::Message::get_second()->get_button(0)->get_width() / 2), neroshop::Message::get_second()->get_height() - neroshop::Message::get_second()->get_button(0)->get_height() - 20);//200-30(height)-20(bottompadding) = 150(button_y)
            // show message box, labels and buttons
            neroshop::Message::get_second()->get_label(1)->show();
            neroshop::Message::get_second()->get_button(0)->show();
            neroshop::Message::get_second()->show();*/
        }
}
////////////////////
void neroshop::Wallet::on_balances_changed(uint64_t new_balance, uint64_t new_unlocked_balance) {
        // if wallet is not fully synced with a daemon, skip this function
        if(!monero_wallet_obj.get()) return;
        if(!monero_wallet_obj.get()->is_synced()) return;
        double piconero = 0.000000000001;
        double balance = (new_balance * piconero);
        double unlocked_balance = (new_unlocked_balance * piconero);
        // if total balance is still locked, display it
        // but if total balance is fully unlocked, then you already have the balance so no need to keep displaying it 
        if(unlocked_balance != balance) {
            // balance updated (unlocked)
            std::cout << std::fixed << std::setprecision(12) << "\033[1;33;49m" << "balance: " << "\033[0m" << balance << std::fixed << std::setprecision(2);// << std::endl;
            std::cout << std::fixed << std::setprecision(12) << " (unlocked_balance: " << unlocked_balance << std::fixed << std::setprecision(2) << ")" << std::endl;
        }
        if(unlocked_balance == balance) {
            std::cout << "\033[1;35;49m" << "Balance is now fully unlocked" << "\033[0m" << std::endl;// Your full balance can be spent now
            //std::cout << std::fixed << std::setprecision(12) << "balance: " << balance << std::fixed << std::setprecision(2);// << std::endl;
            //std::cout << std::fixed << std::setprecision(12) << " (unlocked_balance: " << unlocked_balance << std::fixed << std::setprecision(2) << ")" << std::endl;//Japanese violet: 91, 50, 86
        }
}
////////////////////
////////////////////
////////////////////
// daemon
////////////////////
// open the daemon before opening the wallet
void neroshop::Wallet::daemon_open(const std::string& daemon_dir, bool confirm_external_bind, bool restricted_rpc, std::string data_dir, unsigned int restore_height) 
{
    // Todo: use QProcess to launch monerod
    // Check if there is another monerod process running in the background first
    int monerod = Process::get_process_by_name("monerod");//(argv[1]);//cout << "pid: " << monerod << endl;
    if(monerod != -1) { std::cout << "\033[1;90;49m" << "monerod is running (ID:" << monerod << ")\033[0m" << std::endl; return; } // daemon that was previously running in the background // exit function
    // search for monero daemon (that is packaged with neroshop executable)
    ////std::string daemon_dir = static_cast<std::string>(std::filesystem::current_path()) + "/external/monero-cpp/external/monero-project/build/release/bin/monerod";
    // If neroshop's built-in Monero daemon does not exist, exit function
    if(!std::filesystem::is_regular_file(daemon_dir)) { std::cout << "monerod not found. Please set the path to monerod or use a remote node instead" << "\n"; return; }
    // if neroshop's daemon exists and remote is set to false //if(File::exists(daemon_dir) && remote == false) {  
	// if no other daemon is running, then use daemon that comes packaged with neroshop executable
    std::string ip = (confirm_external_bind == true) ? "0.0.0.0" : "127.0.0.1";
    std::cout << "\033[1;90;49m" << "daemon found: \"" << daemon_dir << "\"" << "\033[0m" << std::endl;
    std::string program  = daemon_dir;
    std::string args = (" --data-dir=" + data_dir) + (" --rpc-bind-ip=" + ip) + (" --rpc-bind-port=38081");
    if(confirm_external_bind == true) { args = args + " --confirm-external-bind"; }
    if(confirm_external_bind == true && restricted_rpc == true) { args = args + " --restricted-rpc"; }
    if(neroshop::string::lower(this->get_network_type_string()) != "mainnet") args = args + (" --" + neroshop::string::lower(this->get_network_type_string()));
    args = args + (" --detach"); // https://monero.stackexchange.com/questions/12005/what-is-the-difference-between-monerod-detach-and-monerod-non-interactive
    std::cout << "\033[1;95;49m" << "$ " << daemon_dir + args << "\033[0m" << std::endl;
    // start the daemon (monerod) as a new process on launch
	process = std::unique_ptr<Process>(new Process(daemon_dir, args));
    monerod = process->get_handle();
    std::cout << "\033[1;90;49m" << "started monerod (ID:" << monerod << ")\033[0m" << std::endl;
}
////////////////////
bool neroshop::Wallet::daemon_connect_local(const std::string& username, const std::string& password) { // connect to a running daemon (node)
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    // connect to the daemon
	monero_wallet_obj->set_daemon_connection(monero_rpc_connection(std::string("http://127.0.0.1:38081"), username, password));
    std::cout << "\033[1;90;49m" << "waiting for daemon" << "\033[0m" << std::endl;
    bool connected = false; bool synced = false;
    while(!connected) {
        if(monero_wallet_obj.get()->is_connected_to_daemon()) {          
            connected = true;
            // once connected to daemon, listen to sync progress (use this only on a detached daemon)
            std::cout << "\033[1;90;49m" << "sync in progress .." << "\033[0m" << std::endl;
            // it is not safe to connect to a daemon that has not fully synced, so listen to the sync progress before attempting to do anything else
            // wallet height is 938308 (2021-10-07) - will store all txs/balances but takes longer to sync //std::cout << "syncing from wallet height: " << monero_wallet_obj->get_height() << std::endl;
            // daemon height is 1070393 (most recent) - will only store the most recent txs/balances and syncs almost instantly //std::cout << "syncing from daemon height: " << monero_wallet_obj->get_daemon_height() << std::endl;
            // if height is wallet's height then it will sync from the time of the wallet's creation date to the current date which takes longer to sync
            // if height is daemon height, it will sync instantly but balances will not be updated and you will not be able to generate unique addresses
            // if height is zero, then it will sync from 80% but will still take long to sync
            std::packaged_task<void(void)> sync_job([this]() {
                // monero_wallet_obj->get_daemon_height() is the height that the wallet's daemon is currently synced to
                monero_wallet_obj->sync(monero_wallet_obj->get_sync_height()/*monero_wallet_obj->get_daemon_height()*/, *this);////monero_wallet_obj->sync(0, *this);// 0 = start_height	is the start height to sync from (ignored if less than last processed block) //(sync_listener);//monero_sync_result sync_result = monero_wallet_obj->sync(sync_listener); // synchronize the wallet with the daemon as a one-time synchronous process//if(sync_result.m_received_money) {neroshop::print(std::string("blocks fetched: ") + std::to_string(sync_result.m_num_blocks_fetched));neroshop::print("you have received money");}
            // continue syncing in order to receive tx notifications
            std::cout << "\033[1;90;49m" << "starting background sync" << "\033[0m" << std::endl;
            monero_wallet_obj->start_syncing(5000); // begin syncing the wallet constantly in the background (every 5 seconds)
            // check if wallet's daemon is synced with the network
            if(monero_wallet_obj.get()->is_daemon_synced()) {
                //synced = true;
                std::cout << "\033[1;90;49m" << "daemon is now fully synced with the network" << "\033[0m" << std::endl;
                // add new listener when done syncing
                monero_wallet_obj->add_listener(*this); // add wallet_listener 
            }   
            });
            std::future<void> job_value = sync_job.get_future();
            // move the task (function) to a separate thread
            std::thread worker(std::move(sync_job));
            worker.detach();//worker.join();       
            // todo: store wallet restore height in database on registration
        }
    }
    return synced;
}
////////////////////
void neroshop::Wallet::daemon_connect_remote(const std::string& ip, const std::string& port, const std::string& username, const std::string& password, const monero_wallet_listener* listener) {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    monero_wallet_obj->set_daemon_connection(monero_rpc_connection(std::string("http://" + ip + ":" + port)));//, username, password));
    if(monero_wallet_obj.get()->is_connected_to_daemon()) {
        std::cout << "\033[1;90;49m" << "connected to daemon" << "\033[0m" << std::endl;
            std::packaged_task<void(void)> sync_job([this, listener]() {
                std::cout << "\033[1;90;49m" << "sync in progress ..." << "\033[0m" << std::endl;
                monero_wallet_obj->sync(monero_wallet_obj->get_sync_height(), (listener != nullptr) ? *const_cast<monero_wallet_listener*>(listener) : *this);//*this); // a start_height of 0 is ignored // get_sync_height() is the height of the first block that the wallet scans// get_daemon_height() is the height that the wallet's daemon is currently synced to (will sync instantly but will not guarantee unique subaddress generation)
                // begin syncing the wallet constantly in the background (every 5 seconds) in order to receive tx notifications
                monero_wallet_obj->start_syncing(5000);
                // check if wallet's daemon is synced with the network
                if(monero_wallet_obj.get()->is_daemon_synced()) {////if(monero_wallet_obj->get_daemon_height() == monero_wallet_obj->get_daemon_max_peer_height()) {
                    std::cout << "\033[1;90;49m" << "daemon is now fully synced with the network" << "\033[0m" << std::endl;
                    // add new wallet_listener when done syncing
                    monero_wallet_obj->add_listener((listener != nullptr) ? *const_cast<monero_wallet_listener*>(listener) : *this);//(*this);
                }   
            });
            std::future<void> job_value = sync_job.get_future();
            // move the task (function) to a separate thread to prevent blocking of the main thread
            std::thread node_worker(std::move(sync_job));
            node_worker.detach();
            // todo: store wallet restore height in database on registration
    }  
}
////////////////////
void neroshop::Wallet::daemon_close() {
    if(!process) throw std::runtime_error("process is not initialized");
#ifdef __gnu_linux__
    int monerod = process->get_handle();
    if(monerod != -1) kill(static_cast<pid_t>(monerod), SIGTERM);
#endif    
    //delete process;
}
////////////////////
////////////////////
void neroshop::Wallet::wallet_info() {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    // wallet must be synced with a daemon in order to get the *most recent* wallet info
    if(!monero_wallet_obj.get()->is_synced()) throw std::runtime_error("wallet is not synced with a daemon");
    // get primary_address, balance, unlocked_balance, etc.
    std::string restored_primary = monero_wallet_obj->get_primary_address(); // same as: monero_wallet_obj->get_account(0, true).m_primary_address.get()
    std::cout << "primary_address: " << restored_primary << std::endl;
    uint64_t balance_raw = monero_wallet_obj->get_balance();    // can specify account and subaddress indices
    double piconero = 0.000000000001; // the smallest unit of a monero (monero has 12 decimal places) // https://web.getmonero.org/resources/moneropedia/denominations.html
    double balance = (double)balance_raw * piconero;
    std::cout << std::fixed << std::setprecision(12) << "balance: " << balance << std::fixed << std::setprecision(2) << std::endl;
    // account ---------------------------------------------------------------
    // account_0 is the default/main account where the primary address derives from
    monero_account account = monero_wallet_obj->get_account(0, true);       // get account with subaddresses
    uint64_t unlocked_balance_raw = account.m_unlocked_balance.get();
    double unlocked_balance = (double)unlocked_balance_raw * piconero;
    std::cout << std::fixed << std::setprecision(12) << "unlocked_balance: " << unlocked_balance << std::fixed << std::setprecision(2) << std::endl; // uint64_t
    // view and spend keys ----------------------------------------------------
    std::cout << "view_key: " << get_view_keys().first << std::endl;
    std::cout << "spend_key: " << "(secret)" << std::endl; // since this is intended to be a view-only wallet
    // subaddress -------------------------------------------------------------
    // generate new subaddresses on the default account 0
    std::cout << std::endl;
    /*for(int i = 0; i < 10; i++) wallet->address_new();
    // get number of subaddresses
    std::cout << std::endl;
    std::cout << "address count: " << wallet->get_address_count() << std::endl;
    std::string last_subaddress = wallet->get_last_subaddress();
    std::cout << "last subaddress created: " << last_subaddress << std::endl;*/
}
////////////////////
////////////////////
// setters
////////////////////
void neroshop::Wallet::set_tx_note(const std::string& txid, const std::string& tx_note) {} // "set_tx_note <txid> [free note text here]" - useful for filling address information
////////////////////
////////////////////
// getters
////////////////////
double neroshop::Wallet::get_sync_percentage() const {
    std::lock_guard<std::mutex> lock(wallet_data_mutex);
    return percentage;
}
////////////////////
unsigned long long neroshop::Wallet::get_sync_height() const {
    std::lock_guard<std::mutex> lock(wallet_data_mutex);
    return height;
}
////////////////////
unsigned long long neroshop::Wallet::get_sync_start_height() const {
    std::lock_guard<std::mutex> lock(wallet_data_mutex);
    return start_height;
}
////////////////////
unsigned long long neroshop::Wallet::get_sync_end_height() const {
    std::lock_guard<std::mutex> lock(wallet_data_mutex);
    return end_height;
}
////////////////////
std::string neroshop::Wallet::get_sync_message() const {
    std::lock_guard<std::mutex> lock(wallet_data_mutex);
    return message;
}
////////////////////
////////////////////
std::string neroshop::Wallet::get_primary_address() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_primary_address();// same as://monero_wallet_obj->get_account(0, true).m_primary_address.get();//monero_account monero::monero_wallet_full::get_account	(	const uint32_t 	account_idx,bool 	include_subaddresses )		//const
}
////////////////////
std::string neroshop::Wallet::get_address(unsigned int index) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_account(0, true).m_subaddresses[index].m_address.get(); // account_idx "0" is default
}
////////////////////
unsigned int neroshop::Wallet::get_address_count() const {  
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_account(0, true).m_subaddresses.size();// all subaddresses will be created on the default account at index 0
}
////////////////////
//////////////////// for some reason, "account.m_subaddresses[i].m_address.get()" fails, but "monero_wallet_obj->get_account(0, true).m_subaddresses.size()" usually succeeds ...-> https://stackoverflow.com/questions/68733975/difficult-to-understand-runtime-error-this-is-initialized-failure-in-boost#comment121472560_68733975
double neroshop::Wallet::get_balance_raw(unsigned int account_index, unsigned int subaddress_index) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    // get balance from subaddress
    if(subaddress_index > 0) return monero_wallet_obj->get_balance(account_index, subaddress_index);//uint64_t monero::monero_wallet_full::get_balance	(	uint32_t 	account_idx, uint32_t 	subaddress_idx )		//const
    // get balance from account (primary address)
    else return monero_wallet_obj->get_balance(account_index);//uint64_t monero::monero_wallet_full::get_balance	(	uint32_t 	account_idx	)	const
    return 0.0;
} // "balance"
////////////////////
double neroshop::Wallet::get_unlocked_balance_raw(unsigned int account_index, unsigned int subaddress_index) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    // get a subaddress's unlocked balance
    if(subaddress_index > 0) return monero_wallet_obj->get_unlocked_balance(account_index, subaddress_index);//uint64_t monero::monero_wallet_full::get_unlocked_balance	(	uint32_t 	account_idx, uint32_t 	subaddress_idx )		const
    // get an account's unlocked balance (primary address)
    else return monero_wallet_obj->get_unlocked_balance(account_index);//uint64_t monero::monero_wallet_full::get_unlocked_balance	(		)	const//uint64_t monero::monero_wallet_full::get_unlocked_balance	(	uint32_t 	account_idx	)	const
    return 0.0;
} // "balance"
////////////////////
double neroshop::Wallet::get_balance(unsigned int account_index, unsigned int subaddress_index) const {
    double piconero = 0.000000000001;
    return get_balance_raw(account_index, subaddress_index) * piconero;
}
////////////////////
double neroshop::Wallet::get_unlocked_balance(unsigned int account_index, unsigned int subaddress_index) const {
    double piconero = 0.000000000001;
    return get_unlocked_balance_raw(account_index, subaddress_index) * piconero;
}
////////////////////
std::vector<std::string> neroshop::Wallet::get_transactions() const {
    std::vector<std::string> txs_list;
    /* ****
    // query incoming transfers to account 0
    /*monero_transfer_query transfer_query;
    transfer_query.m_is_incoming = true;
    transfer_query.m_account_index = 0;
    vector<shared_ptr<monero_transfer>> transfers = monero_wallet_obj->get_transfers(transfer_query);

    // query unspent outputs
    monero_output_query output_query;
    output_query.m_is_spent = false;
    vector<shared_ptr<monero_output_wallet>> outputs = wallet_restored->get_outputs(output_query);

    // query a transaction by hash
    monero_tx_query tx_query;
    tx_query.m_hash = "314a0f1375db31cea4dac4e0a51514a6282b43792269b3660166d4d2b46437ca";
    shared_ptr<monero_tx_wallet> tx = wallet_restored->get_txs(tx_query)[0];
    for (const shared_ptr<monero_transfer> transfer : tx->get_transfers()) {
      bool is_incoming = transfer->is_incoming().get();
      uint64_t in_amount = transfer->m_amount.get();
      int account_index = transfer->m_account_index.get();
    }
    */
    return txs_list;
} // "show_transfers"
////////////////////
unsigned int neroshop::Wallet::get_transactions_count() const {return 0;}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
std::string neroshop::Wallet::get_last_subaddress() const 
{
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    // unless wallet is synced to a daemon, you will not be able to get the *most recent* last subaddress created
    if(!monero_wallet_obj.get()->is_synced()) throw std::runtime_error("wallet is not synced with a daemon");
    unsigned int last = monero_wallet_obj->get_account(0, true).m_subaddresses.size() - 1;
    return monero_wallet_obj->get_account(0, true).m_subaddresses[last].m_address.get();
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
// proof (proving the transaction was submitted) - https://web.getmonero.org/resources/user-guides/prove-payment.html
//std::string neroshop::Wallet::get_tx_note(const std::string& txid) const {return "";} // "get_tx_note <txid>" - useful for retrieving address information
////////////////////
std::string neroshop::Wallet::get_private_view_key() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_private_view_key();
}
////////////////////
std::string neroshop::Wallet::get_public_view_key() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_public_view_key();
}
////////////////////
std::pair<std::string, std::string> neroshop::Wallet::get_view_keys() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return std::make_pair(monero_wallet_obj->get_private_view_key(), monero_wallet_obj->get_public_view_key());
}
////////////////////
std::string neroshop::Wallet::get_private_spend_key() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_private_spend_key();
}
////////////////////
std::string neroshop::Wallet::get_public_spend_key() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_public_spend_key();
}
////////////////////
std::pair<std::string, std::string> neroshop::Wallet::get_spend_keys() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return std::make_pair(monero_wallet_obj->get_private_spend_key(), monero_wallet_obj->get_public_spend_key());
}
////////////////////
std::string neroshop::Wallet::get_mnemonic() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_mnemonic();
}
//-Image *  neroshop::Wallet::get_qr_code() const {} // returns address qrcode // "show_qr_code"
//-Image *  neroshop::Wallet::get_qr_code(unsigned int address_index) const {} // returns the qrcode of the address at "index"
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
std::string neroshop::Wallet::get_path() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->monero_wallet_full::get_path(); // returns the path of this wallet's file on disk (without the .keys ext)
}
////////////////////
std::string neroshop::Wallet::get_description() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return "";
} // "wallet_info"    
////////////////////
std::string neroshop::Wallet::get_type() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return "";
} // "wallet_info": Normal, HW
////////////////////
unsigned int neroshop::Wallet::get_daemon_height() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_daemon_height();
}
////////////////////
unsigned int neroshop::Wallet::get_height() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_height();
}
////////////////////
unsigned int neroshop::Wallet::get_height_by_date(int year, int month, int day) const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return monero_wallet_obj->get_height_by_date(year, month, day);
}
////////////////////
monero::monero_network_type neroshop::Wallet::get_network_type() const {
    if(!monero_wallet_obj.get()) return this->network_type;
    return monero_wallet_obj->get_network_type();
}
////////////////////
std::string neroshop::Wallet::get_network_type_string() const {
    switch(this->get_network_type()) {
        case monero_network_type::MAINNET: return "mainnet"; break; // 0
        case monero_network_type::TESTNET: return "testnet"; break; // 1
        case monero_network_type::STAGENET: return "stagenet"; break; // 2
    }
    return "mainnet";
}
////////////////////
std::string neroshop::Wallet::get_status() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    return "";
} // "status" - Check current status of wallet.
////////////////////
std::string neroshop::Wallet::get_version() const {
    if(!monero_wallet_obj.get()) throw std::runtime_error("monero_wallet_full is not opened");
    //monero_version monero::monero_wallet_full::get_version();
    return "";
} // "version" - Check software version.
////////////////////
monero_wallet_full * neroshop::Wallet::get_monero_wallet() const
{
    return monero_wallet_obj.get();
}
////////////////////
// callbacks
////////////////////
void neroshop::Wallet::load_from_config(std::string/*const std::string&*/ password) // load configs on opening
{

}
////////////////////
////////////////////
//void  neroshop::Wallet::set_daemon() {} // "set_daemon <host>[:<port>] [trusted|untrusted|this-is-probably-a-spy-node]" - connects to a daemon
////////////////////
//void  neroshop::Wallet::refresh() {}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
bool neroshop::Wallet::file_exists(const std::string& filename) const {
    return monero::monero_wallet_full::wallet_exists(filename + ".keys");
}
////////////////////
bool neroshop::Wallet::is_valid_address(const std::string& address) const {
    return monero_utils::is_valid_address(address, this->get_network_type());
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
// void monero::monero_wallet_full::rescan_blockchain	(		) // dangerous! restarts blockchain sync!
// get last subaddress created
//unsigned int last = (monero_wallet_obj->get_account(0, true).m_subaddresses.size() - 1);
//return monero_wallet_obj->get_account(0, true).m_subaddresses[last].m_address.get() << std::endl;
//monero_wallet_obj->move_to (wallet_file, "supersecretpassword123"); // Move the wallet from its current path to the given path.
//std::cout << "\033[1;97;49m" << "moved file \"" << wallet->get_file() << "\" to \"" << wallet_file << "\"\033[0m" << std::endl;
