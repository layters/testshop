// neroshop
#include "../include/neroshop.hpp"
using namespace neroshop;

lua_State * neroshop::lua_state = luaL_newstate();

static void register_user() {
    db::Sqlite3 * database = db::Sqlite3::get_singleton();//std::unique_ptr<db::Sqlite3> database = std::make_unique<db::Sqlite3>();
    if(!database->open("data.sqlite3")) {//(":memory:")) { // In-memory databases are temporary and written in RAM. They do not support WAL mode either
        neroshop::print(SQLITE3_TAG "\033[91mSQLite::open failed");
    }
    //----------------------------
    Wallet * wallet = new Wallet();
    std::string wallet_name;
    std::string wallet_pwd;
    std::string wallet_confirm_pwd;
    std::cout << "Please enter your desired wallet name:\n";
    std::getline(std::cin, wallet_name);
    if(wallet_name.empty()) wallet_name = "auth"; // default wallet name
    std::cout << "Please enter your desired wallet password:\n";
    std::getline(std::cin, wallet_pwd);
    std::cout << "Please confirm your wallet password:\n";
    std::getline(std::cin, wallet_confirm_pwd);
    // Create a directory in home folder for wallets
    // todo: make wallet path customizable by user
    if(!std::filesystem::is_directory(NEROSHOP_DEFAULT_WALLET_PATH)) {
        if(!std::filesystem::create_directories(NEROSHOP_DEFAULT_WALLET_PATH)) {
            neroshop::print("FAILED TO MAKE WALLET DIRECTORY", 1);
            return;
        }
        neroshop::print(std::string("\033[1;97mcreated directory \"") + NEROSHOP_DEFAULT_WALLET_PATH + "\"");
    }
    // Create a random wallet
    wallet->create_random(wallet_pwd, wallet_confirm_pwd, NEROSHOP_DEFAULT_WALLET_PATH + "/" + wallet_name);
    // Provide user with their mnemonic
    std::cout << "Store these words safely. These 25 words are the key to your account. If you lose them, your account goes bye-bye:\n\033[1;36m";
    std::cout << wallet->get_mnemonic() << "\033[0m" << std::endl;
    // And their secret view and spend key pairs too ...
    std::cout << "Secret view key: " << wallet->get_monero_wallet()->get_private_view_key() << std::endl;
    std::cout << "Secret spend key: " << wallet->get_monero_wallet()->get_private_spend_key() << std::endl;
    // Get SHA256 hash of the primary address
    std::string primary_address = wallet->get_monero_wallet()->get_primary_address();
    std::string user_auth_key;// = neroshop::algo::sha256(primary_address);
    Validator::generate_sha256_hash(primary_address, user_auth_key); // temp
    neroshop::print("Primary address: \033[1;33m" + primary_address + "\033[1;37m\nSHA256 hash: \033[1;32m" + user_auth_key);
    // Store auth credentials in database
    // todo: make this command (DB entry) a client request that the server must respond to and the consensus must agree with
    database->execute_params("INSERT INTO users(key) VALUES($1)", { user_auth_key });
}


static bool auth_with_wallet_file() {
    db::Sqlite3 * database = db::Sqlite3::get_singleton();//std::unique_ptr<db::Sqlite3> database = std::make_unique<db::Sqlite3>();
    if(!database->open("data.sqlite3")) {//(":memory:")) { // In-memory databases are temporary and written in RAM. They do not support WAL mode either
        neroshop::print(SQLITE3_TAG "\033[91mSQLite::open failed");
    }
    //----------------------------
    Wallet * wallet = new Wallet();
    // Initialize monero wallet with existing wallet file
    std::string wallet_password;// = "supersecretpassword123"; // Apparently passwords are not used nor required for mnemonics. ONLY wallet files use passwords
    //std::cout << "Please upload your wallet file:\n";
    std::cout << "Please enter your wallet password:\n";
    std::getline(std::cin, wallet_password);
    // Upload wallet via file dialog
    wallet->upload(true, wallet_password);
    // Get the hash of the primary address
    std::string primary_address = wallet->get_monero_wallet()->get_primary_address();
    std::string user_auth_key;// = neroshop::algo::sha256(primary_address);
    Validator::generate_sha256_hash(primary_address, user_auth_key); // temp
    neroshop::print("Primary address: \033[1;33m" + primary_address + "\033[1;37m\nSHA256 hash: " + user_auth_key);
    //$ echo -n "528qdm2pXnYYesCy5VdmBneWeaSZutEijFVAKjpVHeVd4unsCSM55CjgViQsK9WFNHK1eZgcCuZ3fRqYpzKDokqSKp4yp38" | sha256sum
    // Check database to see if user key (hash of primary address) exists
    bool user_key_found = database->get_integer_params("SELECT EXISTS(SELECT * FROM users WHERE key = $1)", { user_auth_key });
    // If user key is not found in the database, then create one. This is like registering for an account
    if(!user_key_found) {
        // In reality, this function will return false if user key is not registered in the database
        neroshop::print("user key not found in database. Please try again or register", 1);
        return false;
    }
    // Save user information in memory
    int user_id = database->get_integer_params("SELECT id FROM users WHERE key = $1", { user_auth_key });
    // This number will scale as the user count grows
    int min_digits = 15; // 15 digits = 100 trillionth place (000,000,000,000,000)
    int precision = min_digits - std::min<int>(min_digits, std::to_string(user_id).size());
    std::string formatted_user_id = std::string(precision, '0').append(std::to_string(user_id));
    neroshop::print("Welcome back, user " + formatted_user_id, 4);
    // Set user_id
    // ...    
    return true;
}


static bool auth_with_seed() {
    db::Sqlite3 * database = db::Sqlite3::get_singleton();//std::unique_ptr<db::Sqlite3> database = std::make_unique<db::Sqlite3>();
    if(!database->open("data.sqlite3")) {//(":memory:")) { // In-memory databases are temporary and written in RAM. They do not support WAL mode either
        neroshop::print(SQLITE3_TAG "\033[91mSQLite::open failed");
    }
    //----------------------------
    Wallet * wallet = new Wallet();
    // Initialize monero wallet with existing wallet mnemonic
    std::string wallet_mnemonic;// = "hefty value later extra artistic firm radar yodel talent future fungal nutshell because sanity awesome nail unjustly rage unafraid cedar delayed thumbs comb custom sanity";
    std::string wallet_password;// = "supersecretpassword123"; // Apparently passwords are not used nor required for mnemonics. ONLY wallet files use passwords
    std::cout << "Please enter your wallet mnemonic:\n";
    std::getline(std::cin, wallet_mnemonic);
    // todo: allow user to specify a custom location for the wallet keyfile or use a default location
    wallet->restore_from_mnemonic(wallet_mnemonic);
    // Get the hash of the primary address
    std::string primary_address = wallet->get_monero_wallet()->get_primary_address();
    std::string user_auth_key;// = neroshop::algo::sha256(primary_address);
    Validator::generate_sha256_hash(primary_address, user_auth_key); // temp
    neroshop::print("Primary address: \033[1;33m" + primary_address + "\033[1;37m\nSHA256 hash: " + user_auth_key);
    //$ echo -n "528qdm2pXnYYesCy5VdmBneWeaSZutEijFVAKjpVHeVd4unsCSM55CjgViQsK9WFNHK1eZgcCuZ3fRqYpzKDokqSKp4yp38" | sha256sum
    // Check database to see if user key (hash of primary address) exists
    bool user_key_found = database->get_integer_params("SELECT EXISTS(SELECT * FROM users WHERE key = $1)", { user_auth_key });
    // If user key is not found in the database, then create one. This is like registering for an account
    if(!user_key_found) {
        // In reality, this function will return false if user key is not registered in the database
        neroshop::print("user key not found in database. Please try again or register", 1);
        return false;
    }
    // Save user information in memory
    int user_id = database->get_integer_params("SELECT id FROM users WHERE key = $1", { user_auth_key });
    // This number will scale as the user count grows
    int min_digits = 15; // 15 digits = 100 trillionth place (000,000,000,000,000)
    int precision = min_digits - std::min<int>(min_digits, std::to_string(user_id).size());
    std::string formatted_user_id = std::string(precision, '0').append(std::to_string(user_id));
    neroshop::print("Welcome back, user " + formatted_user_id, 4);
    // Set user_id
    // ...    
    return true;
}


static bool auth_with_keys() {
    db::Sqlite3 * database = db::Sqlite3::get_singleton();//std::unique_ptr<db::Sqlite3> database = std::make_unique<db::Sqlite3>();
    if(!database->open("data.sqlite3")) {//(":memory:")) { // In-memory databases are temporary and written in RAM. They do not support WAL mode either
        neroshop::print(SQLITE3_TAG "\033[91mSQLite::open failed");
    }
    //----------------------------
    Wallet * wallet = new Wallet();
    // Initialize monero wallet with existing wallet mnemonic
    std::string primary_address;
    std::string secret_view_key;
    std::string secret_spend_key;
    std::cout << "Please enter your primary address:\n";
    std::getline(std::cin, primary_address);
    std::cout << "Please enter your secret view key:\n";
    std::getline(std::cin, secret_view_key);
    std::cout << "Please enter your secret spend key (optional):\n";
    std::getline(std::cin, secret_spend_key);
    // todo: allow user to specify a custom location for the wallet keyfile or use a default location
    wallet->restore_from_keys(primary_address, secret_view_key, secret_spend_key);
    // Get the hash of the primary address
    std::string user_auth_key;// = neroshop::algo::sha256(primary_address);
    Validator::generate_sha256_hash(primary_address, user_auth_key); // temp
    neroshop::print("Primary address: \033[1;33m" + primary_address + "\033[1;37m\nSHA256 hash: " + user_auth_key);
    //$ echo -n "528qdm2pXnYYesCy5VdmBneWeaSZutEijFVAKjpVHeVd4unsCSM55CjgViQsK9WFNHK1eZgcCuZ3fRqYpzKDokqSKp4yp38" | sha256sum
    // Check database to see if user key (hash of primary address) exists
    bool user_key_found = database->get_integer_params("SELECT EXISTS(SELECT * FROM users WHERE key = $1)", { user_auth_key });
    // If user key is not found in the database, then create one. This is like registering for an account
    if(!user_key_found) {
        // In reality, this function will return false if user key is not registered in the database
        neroshop::print("user key not found in database. Please try again or register", 1);
        return false;
    }
    // Save user information in memory
    int user_id = database->get_integer_params("SELECT id FROM users WHERE key = $1", { user_auth_key });
    // This number will scale as the user count grows
    int min_digits = 15; // 15 digits = 100 trillionth place (000,000,000,000,000)
    int precision = min_digits - std::min<int>(min_digits, std::to_string(user_id).size());
    std::string formatted_user_id = std::string(precision, '0').append(std::to_string(user_id));
    neroshop::print("Welcome back, user " + formatted_user_id, 4);
    // Set user_id
    // ...
    return true;
}

int main() {
    //register_user();
    //auth_with_seed();
    //auth_with_wallet_file();
    auth_with_keys();
    return 0;
}
