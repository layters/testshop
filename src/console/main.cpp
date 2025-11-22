#include <iostream>
#include <string>

#include "../neroshop.hpp"
using namespace neroshop;

#include <linenoise.h>

static void print_commands() {
    std::map<std::string, std::string> commands { // std::map sorts command names in alphabetical order
        {"help             ", "Display list of available commands"},
        {"exit             ", "Exit CLI"},
        {"version          ", "Show version"},
        {"monero_nodes     ", "Display a list of monero nodes"},
        {"query            ", "Execute an SQLite query"},
        {"curl_version     ", "Show libcurl version"},
        {"get              ", "Get the value for a key from DHT network"},
        {"status           ", "Get network status from local daemon server"},
        {"create_wallet    ", "Create a new Monero wallet"},
        {"generate_rsa_keys", "Generate RSA key pairs"},
        {"register         ", "Register an account in the DHT network"}
        //,{"", ""}
    };
    std::cout << "Usage: " << "[COMMAND] ...\n\n";
    for (auto const& [key, value] : commands) {
        std::cout << "  " << key << "  " << value << "\n";
    }
    std::cout << "\n";
}

int main(int argc, char** argv) {
    //-------------------------------------------------------
    // Connect to daemon server (daemon must be launched first)
    neroshop::Client * client = neroshop::Client::get_main_client();
	int port = NEROSHOP_IPC_DEFAULT_PORT;
	std::string addr = "localhost"; // 0.0.0.0 means anyone can connect to your server
	if(!client->connect(port, addr)) {
	    std::cout << "Please launch neroshopd first\n";
	    exit(0);
	}
    //-------------------------------------------------------
    neroshop::load_nodes_from_memory();

    std::vector<std::string> networks = {"mainnet", "testnet", "stagenet"};

    std::string network_type = Wallet::get_network_type_as_string();
    if (std::find(networks.begin(), networks.end(), network_type) == networks.end()) {
        log_error("{}network_type \"{}\" is not valid{}", "\033[1;91m", network_type, color_reset);
        return 1;
    }
    std::vector<std::string> monero_nodes = Script::get_table_string(lua_state, "monero.nodes." + network_type);
    //-------------------------------------------------------
    Wallet wallet(WalletType::Monero);
    Seller user;
    //-------------------------------------------------------
    char * line = NULL;
    while((line = linenoise("neroshop-console> ")) != NULL) {
        std::string command { line };

        linenoiseHistoryLoad("history.txt");
        linenoiseHistoryAdd(line); // Add to the history.
        linenoiseHistorySave("history.txt"); // Save the history on disk.

        if(command == "help" || !strncmp(line, "\0", command.length())) { // By default, pressing "Enter" on an empty string will execute this code
            print_commands();
        }  
        else if(command == "monero_nodes") {
            for(std::string nodes : monero_nodes) {
                std::cout << "\033[1;36m" << nodes << "\033[0m" << std::endl;
            }        
        }            
        else if(command == "version") {
            std::cout << "\033[0;93m" << NEROSHOP_VERSION_FULL << "\033[0m" << std::endl;
        }         
        else if(neroshop::string_tools::starts_with(command, "query")) {
            auto arg_count = neroshop::string_tools::split(command, " ").size();
            if(arg_count == 1) std::cout << "\033[91mexpected arguments after 'query'\033[0m\n";
            if(arg_count > 1) { 
                std::size_t arg_pos = command.find_first_of(" ");
                std::string sql = neroshop::string_tools::trim_left(command.substr(arg_pos + 1));////trim_left(command.substr(std::string("query").length() + 1)); // <- this works too
                assert(neroshop::string_tools::starts_with(sql, "SELECT", false) && "Only SELECT queries are allowed"); // since the ability to run sql commands gives too much power to the user to alter the database anyhow, limit queries to select statements only
                std::string json = neroshop::rpc::json_translate(sql);
                // No need to call RPC server to access RPC functions when it can be done directly.
                std::cout << neroshop::rpc::json_process(json) << "\n";
                // Usage: query SELECT * FROM mappings;
            }
        }
        else if(command == "status") {
            if (client->is_connected()) {
                std::string response;
                client->get("status", response);
            
                try {
                    nlohmann::json json = nlohmann::json::parse(response);
                    std::cout << json.dump(4) << "\033[0m\n";
                } catch (const nlohmann::detail::parse_error& e) {
                    std::cerr << "Parse error\n";
                }
            }
        }
        else if(command == "curl_version") {
            curl_version_info_data * curl_version = curl_version_info(CURLVERSION_NOW);
            std::string curl_version_str = std::to_string((curl_version->version_num >> 16) & 0xff) + 
                "." + std::to_string((curl_version->version_num >> 8) & 0xff) + 
                "." + std::to_string(curl_version->version_num & 0xff);
            std::cout << "libcurl version " << curl_version_str << std::endl;
        }
        else if(neroshop::string_tools::starts_with(command, "get")) {
            auto arg_count = neroshop::string_tools::split(command, " ").size();
            if(arg_count == 1) std::cout << "\033[91mexpected argument after 'get'\033[0m\n";
            if(arg_count > 1) { 
                std::size_t arg_pos = command.find_first_of(" ");
                std::string key = neroshop::string_tools::trim(command.substr(arg_pos + 1));
                
                if (client->is_connected()) {
                    std::string response;
                    client->get(key, response);
                    
                    try {
                        nlohmann::json json = nlohmann::json::parse(response);
                        std::cout << (json.contains("error") ? "\033[91m" : "\033[32m") << json.dump(4) << "\033[0m\n";
                    } catch (const nlohmann::detail::parse_error& e) {
                        std::cerr << "Parse error\n";
                    }
                }
            }
        }     
        else if(command == "register") {
            std::string display_name;
            std::string monero_address;
            std::string public_key;
            std::string private_key;
            // No avatars for CLI (yet)
            
            if(!wallet.is_opened()) { 
                std::cerr << "\033[91mPlease create a wallet before registering an account using the 'create_wallet' command\033[0m\n";
                continue;
            }
            
            if(user.get_public_key().empty()) {
                std::cerr << "\033[91mPlease generate your RSA keys before registering an account using the `generate_rsa_keys` command\033[0m\n";
                continue;
            }
                
            std::cout << "Enter display name (optional): ";
            std::getline(std::cin, display_name);
            if(!display_name.empty()) {
                if(!neroshop::string_tools::is_valid_username(display_name)) {
                    std::cerr << "\033[91mNot a valid display name: " + display_name << "\033[0m\n";
                    continue;
                }
                user.set_name(display_name);
            }
            
            user.set_id(user.get_wallet()->get_primary_address());
            // TODO: create a cart for user in database
            
            auto data = neroshop::Serializer::serialize(user);
            std::string key = data.first;
            std::string value = data.second;
    
            // Send put and receive response
            std::string response;
            client->put(key, value, response);
            std::cout << "Received response: " << response << "\n";
        }
        else if(command == "login") {
        }
        else if(command == "create_wallet") {
            if(wallet.is_opened()) { 
                std::cerr << "\033[91mWallet is already opened" << "\033[0m\n"; 
                continue;
            }
            std::string password;
            std::string confirm_pwd;
            std::string wallet_name;
            std::string wallet_path;
            
            std::cout << "Enter wallet name: ";
            std::getline(std::cin, wallet_name);
            if(wallet_name.empty()) { wallet_name = "wallet"; }
            
            /*std::cout << "Enter wallet path (optional): "; // Can be empty
            std::getline(std::cin, wallet_path);
            if(!wallet_path.empty()) {
                wallet_path = wallet_path + "/" + wallet_name; // Fails for some reason
            }*/
            if(wallet_path.empty()) { wallet_path = neroshop::get_default_wallet_path() + "/" + wallet_name; }
            
            std::cout << "Enter wallet password: ";
            std::getline(std::cin, password);
            
            std::cout << "Confirm wallet password: ";
            std::getline(std::cin, confirm_pwd);
            
            auto wallet_error = wallet.create_random(password, confirm_pwd, wallet_path);
            if(wallet_error == 0) {
                std::cout << std::endl;
                std::cout << "Wallet address: " << wallet.get_primary_address() << std::endl;
                std::cout << "Balance (not synced): " << std::fixed << std::setprecision(12) << wallet.get_balance() << std::fixed << std::setprecision(2) << std::endl;
                std::cout << "Unlocked balance (not synced): " << std::fixed << std::setprecision(12) << wallet.get_unlocked_balance() << std::fixed << std::setprecision(2) << std::endl;
                std::cout << std::endl;
                std::cout << "Mnemonic phrase: " << wallet.get_seed() << std::endl;
                std::cout << std::endl;
                auto view_keys = wallet.get_view_keys();
                std::cout << "View keys: \n" << view_keys.first << " (private)\n" << view_keys.second << " (public)" << std::endl;
                std::cout << std::endl;
                auto spend_keys = wallet.get_spend_keys();
                std::cout << "Spend keys: \n" << spend_keys.first << " (private)\n" << spend_keys.second << " (public)" << std::endl;
                //std::cout << ": " << wallet.get_() << std::endl;
                user.set_wallet(&wallet);
            }
        }
        else if(command == "generate_rsa_keys") {
            std::string user_id;
            
            if(!user.get_public_key().empty()) {
                std::cerr << "\033[91mRSA keys were already generated\033[0m\n";
                continue;
            }
            
            if(!wallet.is_opened()) { 
                std::cerr << "\033[91mPlease create a wallet before generating RSA keys using the 'create_wallet' command\033[0m\n";
                continue;
            }
            user_id = wallet.get_primary_address();
            
            std::string public_key_filename = neroshop::get_default_keys_path() + "/" + user_id + ".pub";
            std::string private_key_filename = neroshop::get_default_keys_path() + "/" + user_id + ".key";
            EVP_PKEY * pkey = neroshop::crypto::rsa_generate_keys_get();
            if(pkey != nullptr) {
                // Get a copy of the public key
                std::string public_key = neroshop::crypto::rsa_get_public_key(pkey);
                std::string private_key = neroshop::crypto::rsa_get_private_key(pkey);
                // Save the key pair to disk
                if(!neroshop::crypto::rsa_save_keys(pkey, public_key_filename, private_key_filename)) {
                    std::cerr << "\033[91mFailed to save RSA key pair" << "\033[0m\n";
                    continue;
                }
                // Print RSA keys so user can copy them
                std::cout << std::endl;
                std::cout << public_key << std::endl;
                std::cout << std::endl;
                std::cout << private_key << std::endl;
                // Set user's RSA keys
                user.set_public_key(public_key);
                user.set_private_key(private_key);
            } else {
                std::cerr << "\033[91mFailed to generate RSA key pair" << "\033[0m\n";
            }
        }
        /*else if(command == "") {
        }*/        
        else if(command == "exit") {
            // close the connection
            client->disconnect();
            break;
        }
        else {
            std::cerr << std::string("\033[0;91mUnrecognized command: \033[1;37m") << command << "\033[0m" << std::endl;  
        }
        linenoiseFree(line); // Or just free(line) if you use libc malloc.
    }
    return 0;
}
