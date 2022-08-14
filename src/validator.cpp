#include "validator.hpp"

////////////////////
////////////////////
////////////////////
////////////////////
bool neroshop::Validator::register_user(const std::string& username, const std::string& password, const std::string& confirm_pw, std::string opt_email) // user would have to confirm their pw twice to make sure they match :O - I totally forgot!
{
#if defined(NEROSHOP_USE_LIBBCRYPT)
    // validate username (will appear only in lower-case letters within the app)
    if(!validate_username(username)) return false;
    // validate password
    if(!validate_password(password)) return false;
    if(password != confirm_pw){NEROSHOP_TAG_OUT std::cout << "\033[0;31;49m" << "Both passwords do not match. Try again" << "\033[0m" << std::endl;return false;}
    // if email is opted, validate email
    if(!opt_email.empty() && !validate_email(opt_email)) return false;
    // generate bcrypt salt (random, with a workfactor of 12 or higher)
	char salt[BCRYPT_HASHSIZE];//char * salt;
    if(!generate_bcrypt_salt(12, salt)) {return false;} // 12=1s, 14=4s, 16=16s, 18=64s
    // pre-hash password with sha256 (to circumvent bcrypt's pw length restrictions [55-72])
    std::string pw_prehash;
    if(!generate_sha256_hash(password, pw_prehash)) {return false;}
    // generate bcrypt hash (from pre-hashed password and salt combination)
    char pw_hash[BCRYPT_HASHSIZE];//char * pw_hash;
    if(!generate_bcrypt_hash(pw_prehash, salt, pw_hash)) {return false;}//if(!generate_bcrypt_hash(password, salt, pw_hash)) {return false;}
    // generate sha256 hash (from email)(this is to hide the email from prying eyes, in case of a hack)
    std::string email_hash;
    if(!generate_sha256_hash(opt_email, email_hash)) {return false;}
    // saving (save user information to database)
    save_user(username, pw_hash, (!opt_email.empty()) ? email_hash : "");
#ifdef NEROSHOP_DEBUG
	auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	std::string format = "%Y-%m-%d  %H:%M:%S %p";
	ss << std::put_time(std::localtime(&in_time_t), format.c_str());
    NEROSHOP_TAG_OUT std::cout << ss.str() << "\033[1;32;49m" << " account registered" << "\033[0m" << std::endl;
#endif    
    neroshop::print((!username.empty()) ? std::string("Welcome to neroshop, " + username) : "Welcome to neroshop", 4);
#endif // NEROSHOP_USE_LIBBCRYPT
#if !defined(NEROSHOP_USE_LIBBCRYPT)
    neroshop::print("NEROSHOP_USE_LIBBCRYPT not defined", 1);
    return false;
#endif
    return true;
} //ELI5: https://auth0.com/blog/adding-salt-to-hashing-a-better-way-to-store-passwords/  why bcrypt: https://security.stackexchange.com/questions/4781/do-any-security-experts-recommend-bcrypt-for-password-storage/6415#6415
////////////////////
void neroshop::Validator::save_user(const std::string& username, const char * pw_hash/*const char pw_hash[BCRYPT_HASHSIZE]*/, std::string email_hash) 
{
    ////////////////////////////////
    // sqlite - save a copy of user's account details locally (on user's device)
    ////////////////////////////////
    /*DB::Sqlite3 db("neroshop.db");
    // find a way to check whether database is locked so we know if user is registered or not // Register A Callback To Handle SQLITE_BUSY Errors
    //db.execute("PRAGMA journal_mode = WAL;"); // this may reduce the incidence of SQLITE_BUSY errors (such as database being locked) //https://www.sqlite.org/pragma.html#pragma_journal_mode //https://github.com/mapbox/node-sqlite3/issues/9 //sqlite3_busy_timeout(db.get_handle(), 0);// arg of 0 or negative turns off all busy handlers//returns int
    // create table Role if it does not yet exist
    if(!db.table_exists("role")) {
        db.table("role");
        db.column("role", "ADD", "role_name", "TEXT");
        // these rows are only inserted once since roles are static and do not change // 2 roles (1=buyer, 2=seller)
        db.insert("role", "role_name", DB::Sqlite3::to_sql_string("Buyer"));
        db.insert("role", "role_name", DB::Sqlite3::to_sql_string("Seller"));
    }    
    // create table of Users
    if(!db.table_exists("users")) {
        db.table("users");
        db.column("users", "ADD", "name", "TEXT");
        db.column("users", "ADD", "pw_hash", "TEXT");
        db.column("users", "ADD", "email", "TEXT");
        db.column("users", "ADD", "role_id", "INTEGER");//db.column("users", "ADD", "role", "TEXT");//db.column("users", "ADD", "created_at", "TEXT");
        db.execute("CREATE UNIQUE INDEX idx_users_name ON Users (name);");// enforce that the user names are unique, in case there is an attempt to insert a new "name" of the same value - https://www.sqlitetutorial.net/sqlite-index/
    }
    db.insert("users", "name, pw_hash, email, role_id", DB::Sqlite3::to_sql_string(String::lower(username)) + ", " + DB::Sqlite3::to_sql_string(pw_hash) + ", " + DB::Sqlite3::to_sql_string(email_hash) + ", " + std::to_string(1)); // usernames are stored in lowercase
    db.close();*/ // don't forget to close the db after you're done :)
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");////DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    // begin transaction
    DB::Postgres::get_singleton()->execute("BEGIN;");
    // create a restore point - not necessary
    ////DB::Postgres::get_singleton()->execute("SAVEPOINT before_account_creation_savepoint;");
    ////////////////////////////////
    // table: account_type
    if(!DB::Postgres::get_singleton()->table_exists("account_type")) {
        DB::Postgres::get_singleton()->create_table("account_type");
        DB::Postgres::get_singleton()->add_column("account_type", "name", "text");
        // these rows are only inserted once since roles are static and do not change // 2 roles (1=buyer, 2=seller)
        DB::Postgres::get_singleton()->insert_into("account_type", "name", "'Buyer'");
        DB::Postgres::get_singleton()->insert_into("account_type", "name", "'Seller'");
    }
    ////////////////////////////////    
    // table: users
    if(!DB::Postgres::get_singleton()->table_exists("users")) {
        DB::Postgres::get_singleton()->create_table("users");
        DB::Postgres::get_singleton()->add_column("users", "name", "text");
        DB::Postgres::get_singleton()->add_column("users", "pw_hash", "text");
        DB::Postgres::get_singleton()->add_column("users", "opt_email", "text DEFAULT '' NOT null");
        DB::Postgres::get_singleton()->add_column("users", "account_type_id", "integer REFERENCES account_type(id)"); //  foreign key is a column or a group of columns in a table that reference the primary key of another table // testing invalid foreign key: DB::Postgres::get_singleton()->execute("INSERT INTO users (name, pw_hash, opt_email, account_type_id) VALUES('jack', 'null', 'null', 3) "); // acount_type_id 3 does not exist :O, so this should trigger an error :}
        // usernames and emails MUST be unique!
        DB::Postgres::get_singleton()->execute("CREATE UNIQUE INDEX index_users ON users (name, opt_email);");
    }
    // insert new data into table users
    int user_id = DB::Postgres::get_singleton()->get_integer_params("INSERT INTO users (name, pw_hash, opt_email, account_type_id) VALUES ($1, $2, $3, $4) RETURNING id", { String::lower(username), pw_hash, email_hash, std::to_string(1) });//DB::Postgres::get_singleton()->execute_params("INSERT INTO users (name, pw_hash, opt_email, account_type_id) VALUES ($1, $2, $3, $4)", { String::lower(username), pw_hash, email_hash, std::to_string(1) });
	if(user_id < 1) {
	    DB::Postgres::get_singleton()->execute("ROLLBACK;"); // abort transaction
	    return; // exit function
	}
	//std::cout << "Validator::register_user(): user with id: " << user_id << " has just been created" << std::endl;
	////////////////////////////////
	// table: cart
	if(!DB::Postgres::get_singleton()->table_exists("cart")) {
	    DB::Postgres::get_singleton()->create_table("cart");
	    DB::Postgres::get_singleton()->execute("ALTER TABLE cart ADD COLUMN user_id integer REFERENCES users(id);");
	    // If user is a guest, then their id (user_id) will be 0 but their session_id will be unique
	    //DB::Postgres::get_singleton()->execute("ALTER TABLE cart ADD COLUMN session_id uuid REFERENCES guests(id);"); // for guests
	}
	// insert new data into table cart - each user will have their own cart
	DB::Postgres::get_singleton()->execute_params("INSERT INTO cart (user_id) VALUES ($1)", { std::to_string(user_id) });
	////////////////////////////////
	// table: favorites
	if(!DB::Postgres::get_singleton()->table_exists("favorites")) {
	    DB::Postgres::get_singleton()->create_table("favorites");
	    DB::Postgres::get_singleton()->execute("ALTER TABLE favorites ADD COLUMN user_id integer REFERENCES users(id);");
	    DB::Postgres::get_singleton()->execute("ALTER TABLE favorites ADD COLUMN item_ids integer[];");//REFERENCES item(id);"); // no support for foreign keys on array elements sadly: https://stackoverflow.com/questions/41054507/postgresql-array-of-elements-that-each-are-a-foreign-key
	}
	// insert new data into table favorites
	DB::Postgres::get_singleton()->execute_params("INSERT INTO favorites (user_id, item_ids) VALUES ($1, '{}')", { std::to_string(user_id) });
	////////////////////////////////
	// end transaction
	DB::Postgres::get_singleton()->execute("COMMIT;");
    ////DB::Postgres::get_singleton()->finish();
    ////////////////////////////////
#endif    
}
////////////////////
bool neroshop::Validator::login(const std::string& username, const std::string& password)
{
    ////////////////////////////////
	// sqlite 
    ////////////////////////////////
    /*DB::Sqlite3 db("neroshop.db");
	if(!db.table_exists("users")) {NEROSHOP_TAG_OUT std::cout << "\033[0;33;49m" << "This account is not registered" << "\033[0m" << std::endl; return false;}
	// SELECT pw_hash from Users WHERE name=username
	std::string pw_hash = db.get_column_text("users", "pw_hash", "name = " + DB::Sqlite3::to_sql_string(String::lower(username)));
	// if no pw_hash could be found then the user does not exist (or the db is missing or corrupted)
	if(pw_hash.empty()) { NEROSHOP_TAG_OUT std::cout << "\033[0;33;49m" << "This user does not exist" << "\033[0m" << std::endl; return false; }
	db.close();*/ // don't forget to close the db after you're done :)
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    if(username.empty()) return false; // exit if username is empty -.-
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    if(!DB::Postgres::get_singleton()->table_exists("users")) {NEROSHOP_TAG_OUT std::cout << "\033[0;33;49m" << "This account is not registered" << "\033[0m" << std::endl; /*DB::Postgres::get_singleton()->finish();*/ return false;}
    // SELECT pw_hash from users WHERE name=username
    std::string pw_hash = DB::Postgres::get_singleton()->get_text_params("SELECT pw_hash FROM users WHERE name = $1;", { String::lower(username) });//DB::Postgres::get_singleton()->get_text("SELECT pw_hash FROM users WHERE name = " + DB::Postgres::to_psql_string(String::lower(username)) + ";");//std::cout << "pw_hash: " << pw_hash << " (retrieved from database)\n";
	// if no pw_hash could be found then the user does not exist (or the db is missing or corrupted)
	if(pw_hash.empty()) { NEROSHOP_TAG_OUT std::cout << "\033[0;33;49m" << "This user does not exist" << "\033[0m" << std::endl; /*DB::Postgres::get_singleton()->finish();*/ return false; }    
    /*DB::Postgres::get_singleton()->finish();*/
    ////////////////////////////////
	// validate the pre-hashed pw
    std::string pw_prehash;
    if(!generate_sha256_hash(password, pw_prehash)) {
        return false;
    }
    #if defined(NEROSHOP_USE_LIBBCRYPT)
    if(!validate_bcrypt_hash(pw_prehash, pw_hash)) { //if(!validate_bcrypt_hash(password, pw_hash)) {
        return false;
    }
    #endif
    #if !defined(NEROSHOP_USE_LIBBCRYPT)
    neroshop::print("NEROSHOP_USE_LIBBCRYPT not defined", 1);
    return false;
    #endif
#ifdef NEROSHOP_DEBUG
	auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	std::string format = "%Y-%m-%d  %H:%M:%S %p";
	ss << std::put_time(std::localtime(&in_time_t), format.c_str());
    NEROSHOP_TAG_OUT std::cout << ss.str() << "\033[1;32;49m" << " successfully logged in" << "\033[0m" << std::endl;
#endif
    neroshop::print((!username.empty()) ? std::string("Welcome back, " + username) : "Welcome back", 4);
	return true; // default value
#endif
    return false;	
}
////////////////////
bool neroshop::Validator::login_with_email(const std::string& email, const std::string& password) { // works! but won't be used since most users won't opt-in to use an email
    ////////////////////////////////
    // sqlite
    ////////////////////////////////
    /*DB::Sqlite3 db("neroshop.db");
	if(!db.table_exists("users")) {NEROSHOP_TAG_OUT std::cout << "\033[0;33;49m" << "This account is not registered" << "\033[0m" << std::endl; return false;}
	// get email_hash (from email)
	std::string email_hash;
	if(!generate_sha256_hash(email, email_hash)) { neroshop::print("An error occured", 1); return false;}
	// SELECT pw_hash from Users WHERE email_hash=email_hash
	std::string pw_hash = db.get_column_text("users", "pw_hash", "email = " + DB::Sqlite3::to_sql_string(email_hash));
	// if no pw_hash could be found then the user does not exist (or the db is missing or corrupted)
	if(pw_hash.empty()) { NEROSHOP_TAG_OUT std::cout << "\033[0;33;49m" << "This user does not exist" << "\033[0m" << std::endl; return false; }
	// also, get username from db (only for display purposes)
	std::string username = db.get_column_text("users", "name", "email = " + DB::Sqlite3::to_sql_string(email_hash));
	db.close();*/ // don't forget to close the db after you're done :)
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    if(!DB::Postgres::get_singleton()->table_exists("users")) {NEROSHOP_TAG_OUT std::cout << "\033[0;33;49m" << "This account is not registered" << "\033[0m" << std::endl; /*DB::Postgres::get_singleton()->finish();*/ return false;}
	// get email_hash (from email)
	std::string email_hash;
	if(!generate_sha256_hash(email, email_hash)) { neroshop::print("An error occured", 1); /*DB::Postgres::get_singleton()->finish();*/ return false;}
	// SELECT pw_hash from users WHERE opt_email = email_hash
	std::string pw_hash = DB::Postgres::get_singleton()->get_text_params("SELECT pw_hash FROM users WHERE opt_email = $1;", { email_hash });//+ DB::Postgres::to_psql_string(email_hash));  
    // if no pw_hash could be found then the user does not exist (or the db is missing or corrupted)
    if(pw_hash.empty()) { NEROSHOP_TAG_OUT std::cout << "\033[0;33;49m" << "This user does not exist" << "\033[0m" << std::endl; /*DB::Postgres::get_singleton()->finish();*/ return false; }//std::cout << "pw_hash: " << pw_hash << " (retrieved from db) "/* + username + ")"*/ << std::endl;
    // also, get username from db (only for display purposes)
    std::string username = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM users WHERE opt_email = $1", { email_hash });// + DB::Postgres::to_psql_string(email_hash));
    /*DB::Postgres::get_singleton()->finish();*/
    ////////////////////////////////	
	// validate the pre-hashed pw
    std::string pw_prehash;
    if(!generate_sha256_hash(password, pw_prehash)) {
        return false;
    }
    #if defined(NEROSHOP_USE_LIBBCRYPT)
    if(!validate_bcrypt_hash(pw_prehash, pw_hash)) { //if(!validate_bcrypt_hash(password, pw_hash)) {
        return false;
    }	
    #endif
    #if !defined(NEROSHOP_USE_LIBBCRYPT)
    neroshop::print("NEROSHOP_USE_LIBBCRYPT not defined", 1);
    return false;
    #endif
    // save the raw email within the application (for later use)
#ifdef NEROSHOP_DEBUG
	auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	std::string format = "%Y-%m-%d  %H:%M:%S %p";
	ss << std::put_time(std::localtime(&in_time_t), format.c_str());
    NEROSHOP_TAG_OUT std::cout << ss.str() << "\033[1;32;49m" << " successfully logged in" << "\033[0m" << std::endl;
#endif
    neroshop::print((!username.empty()) ? std::string("Welcome back, " + username) : "Welcome back", 4);
	return true; // default value
#endif
    return false;	
}
////////////////////
void neroshop::Validator::change_pw(const std::string& old_pw, const std::string& new_pw, const std::string& confirm_new_pw) 
{}
////////////////////
////////////////////
////////////////////
bool neroshop::Validator::validate_username(const std::string& username) 
{
    // username (will appear only in lower-case letters within the app)
    // make sure username is at least 2 characters short (min_user_length=2)
    unsigned int min_user_length = 2;
    if(username.length() < min_user_length) {
        NEROSHOP_TAG_OUT std::cout << "Your username must be at least " << min_user_length << " characters in length" << std::endl;
        return false; // exit function
    }
    // make sure username is at least 30 characters long (max_user_length=30)
    unsigned int max_user_length = 30; // what if a company has a long name? :o // also consider the textbox's max-length
    if(username.length() > max_user_length) {
        NEROSHOP_TAG_OUT std::cout << "Your username must be at least " << max_user_length << " characters in length" << std::endl;
        return false; // exit function
    }
    for(int i = 0; i < username.length(); i++)
    {
        // make sure username does not contain any spaces //std::cout << username[i] << " - char\n";
        if(isspace(username[i])) {
            NEROSHOP_TAG_OUT std::cout << "Your username cannot contain any spaces" << std::endl;
            return false; // exit function
        }
        // make sure username can only contain letters, numbers, and these specific symbols: a hyphen, underscore, and period (-,_,.) //https://stackoverflow.com/questions/39819830/what-are-the-allowed-character-in-snapchat-username#comment97381763_41959421
        // symbols like @,#,$,etc. are invalid
        if(!isalnum(username[i])) {
            if(username[i] != '-'){
            if(username[i] != '_'){
            if(username[i] != '.'){
                NEROSHOP_TAG_OUT std::cout << "Your username cannot contain any symbols except (-, _, .): " << username[i] << std::endl;
                return false; // exit function
            }}}
        }
    }
    // make sure username begins with a letter (username cannot start with a symbol or number)
    char first_char = username.at(username.find_first_of(username));
    if(!isalpha(first_char)) {
        NEROSHOP_TAG_OUT std::cout << "Your username must begin with a letter: " << first_char << std::endl;
        return false; // exit function
    }
    // make sure username does not end with a symbol (username can end with a number, but NOT with a symbol)
    char last_char = username.at(username.find_last_of(username));
    if(!isalnum(last_char)) { //if(last_char != '_') { // underscore allowed at end of username? :o
        NEROSHOP_TAG_OUT std::cout << "Your username must end with a letter or number: " << last_char << std::endl;
        return false; // exit function //}
    }
    // the name guest is reserved for guests only, so it cannot be used by any other user
    if(String::lower(username) == "guest") {
        neroshop::print("The name \"Guest\" is reserved for guests ONLY");
        return false;
    }
    ////////////////////////////////
    // sqlite
    ////////////////////////////////
    /*// check db to see if username is not already taken (last thing to worry about)
    DB::Sqlite3 db("neroshop.db");
	if(db.table_exists("users")) 
	{   // make sure table Users exists first
	    std::string name = db.get_column_text("users", "name", "name='" + String::lower(username) + "'");
	    if(name == String::lower(username)) { // names are stored in lowercase
	        neroshop::print("This username is already taken", 2);
	        return false;
	    }//std::cout << "name: " << name << " (retrieved from db)" << std::endl;
	}
	// make sure any deleted user's name is not re-used
	if(db.table_exists("deleted_users")) {
	    std::string deleted_user = db.get_column_text("deleted_users", "name", "name = " + DB::Sqlite3::to_sql_string(username));
	    if(deleted_user == String::lower(username)) {
	        neroshop::print("This username cannot be used", 2);
            return false;
	    }
	}
	db.close();*/ // don't forget to close the db after you're done :)
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    // check db to see if username is not already taken (last thing to worry about)
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
	if(DB::Postgres::get_singleton()->table_exists("users")) 
	{   // make sure table Users exists first
	    std::string name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM users WHERE name = $1;", { String::lower(username) });//+ DB::Postgres::to_psql_string(String::lower(username)));
	    if(name == String::lower(username)) { // names are stored in lowercase
	        neroshop::print("This username is already taken", 2);
	        /*DB::Postgres::get_singleton()->finish();*/
	        return false;
	    }//std::cout << "name: " << name << " (retrieved from db)" << std::endl;
	}    
	// make sure any deleted user's name is not re-used
	if(DB::Postgres::get_singleton()->table_exists("deleted_users")) {
	    std::string deleted_user = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM deleted_users WHERE name = $1;", { username });//+ DB::Postgres::to_psql_string(username));
	    if(deleted_user == String::lower(username)) {
	        neroshop::print("This username cannot be used"/*re-used"*/, 2);
            /*DB::Postgres::get_singleton()->finish();*/
            return false;
	    }
	}    
    /*DB::Postgres::get_singleton()->finish();*/
    ////////////////////////////////    
    return true; // default return value
#endif
    return false;    
}
////////////////////
bool neroshop::Validator::validate_password(const std::string& password) 
{
    //Minimum eight characters, at least one uppercase letter, one lowercase letter, one number and one special character:
    const std::regex pattern("^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[#?!@$%^&*-]).{8,}$"); // source: https://stackoverflow.com/questions/19605150/regex-for-password-must-contain-at-least-eight-characters-at-least-one-number-a
    if(!std::regex_match(password, pattern))
    {
        // figure out the specific reason why password failed to follow the regex rules  //if(password.empty()) { std::cout << "Please enter a valid password" << std::endl; return false; }
        if(!std::regex_search(password.c_str(), std::regex("(?=.*?[A-Z])"))) {
            neroshop::print("Password must have at least one upper case letter", 1);
        }
        if(!std::regex_search(password.c_str(), std::regex("(?=.*?[a-z])"))) {
            neroshop::print("Password must have at least one lower case letter", 1);
        }
        if(!std::regex_search(password.c_str(), std::regex("(?=.*?[0-9])"))) {
            neroshop::print("Password must have at least one digit", 1);
        }              
        if(!std::regex_search(password.c_str(), std::regex("(?=.*?[#?!@$%^&*-])"))) {
            neroshop::print("Password must have at least one special character", 1);
        }         
        if(password.length() < 8) {
            neroshop::print("Password must be at least 8 characters long", 1);
        } 
        //neroshop::print("Please enter a stronger password (at least 1 upper case letter, 1 lower case letter, 1 digit, and 1 special character)", 1);
        return false; // exit function
    }
    return true; // default return value
}
////////////////////
bool neroshop::Validator::validate_email(const std::string& email) {
    // make sure email is a valid email
    const std::regex pattern("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");
    if(!std::regex_match(email, pattern)) {
        neroshop::print("Email address is not valid", 1);
        return false;
    }
    // check db to see if email is not already taken (last thing to worry about)
	// get email_hash (from email)
	std::string email_hash;
	if(!generate_sha256_hash(email, email_hash)) {
	    neroshop::print("Failed to generate SHA256 hash", 1);
	    return false;
	}
    ////////////////////////////////
    // sqlite
    ////////////////////////////////	
    /*DB::Sqlite3 db("neroshop.db");
	if(db.table_exists("users")) {
	    std::string email_taken = db.get_column_text("users", "email", "email = " + DB::Sqlite3::to_sql_string(email_hash));
	    if(email_taken.empty()) { // cannot process an empty string, so exit
	        db.close();
	        return true;
	    }
        // compare the email_hash with the email_taken_hash - if its a match then the email is already in use
        if(email_hash == email_taken) {
            neroshop::print("This email address is already in use", 1);
            db.close();
            return false;
        }
	}
	db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
	if(DB::Postgres::get_singleton()->table_exists("users")) {
	    std::string email_taken = DB::Postgres::get_singleton()->get_text_params("SELECT opt_email FROM users WHERE opt_email = $1;", { email_hash });//+ DB::Postgres::to_psql_string(email_hash));
	    // cannot process an empty string, so close db and exit function
	    if(email_taken.empty()) { /*DB::Postgres::get_singleton()->finish();*/ return true;}
        // compare the email_hash with the email_taken_hash - if its a match then the email is already in use
        if(email_hash == email_taken) {
            neroshop::print("This email address is already in use", 1);
            /*DB::Postgres::get_singleton()->finish();*/
            return false;
        }	    
	}    
    /*DB::Postgres::get_singleton()->finish();*/
    ////////////////////////////////
    return true;
#endif
    return false;    
}
////////////////////
#if defined(NEROSHOP_USE_LIBBCRYPT)
bool neroshop::Validator::validate_bcrypt_hash(const std::string& password, const std::string& hash)
{   
    int result = bcrypt_checkpw(password.c_str(), hash.c_str()); // verify password
#ifdef NEROSHOP_DEBUG
	NEROSHOP_TAG_OUT std::cout << ((result != 0) ? "\033[0;91mThe password does NOT match" : "\033[0;32mThe password matches") << "\033[0m" << std::endl;
#endif
    return (result == 0);
}
////////////////////
bool neroshop::Validator::generate_bcrypt_salt(unsigned int workfactor, char salt[BCRYPT_HASHSIZE])
{   
    int result = bcrypt_gensalt(workfactor, salt); // generate a salt (random) // workfactor must be between 4 and 31 - default is 12
#ifdef NEROSHOP_DEBUG0
    std::cout << ((result != 0) ? NEROSHOP_TAG "bcrypt salt failed to generate" : NEROSHOP_TAG "generated bcrypt salt: " + std::string(salt)) << std::endl; // 0=success
    // two users with the same password results in the same exact hash, which is why a unique salt must be generated
#endif
    return (result == 0);
}
////////////////////
bool neroshop::Validator::generate_bcrypt_hash(const std::string& password, const char salt[BCRYPT_HASHSIZE], char hash[BCRYPT_HASHSIZE])
{   
    int result = bcrypt_hashpw(password.c_str(), salt, hash); // generate hash (from password and salt combination)
#ifdef NEROSHOP_DEBUG0
    std::cout << ((result != 0) ? NEROSHOP_TAG "bcrypt hash failed to generate" : NEROSHOP_TAG "generated bcrypt hash: " + std::string(hash)) << std::endl;
#endif
    return (result == 0);
}
#endif // NEROSHOP_USE_LIBBCRYPT
////////////////////
bool neroshop::Validator::validate_sha256_hash(const std::string& email, const std::string& hash) { // raw/unsalted hash
    std::string temp_hash;
    if(!generate_sha256_hash(email, temp_hash)) return false;
    return (temp_hash == hash);
}
////////////////////
bool neroshop::Validator::generate_sha256_hash(const std::string& email, std::string& hash_out) {
    // EVP (recommended over legacy "SHA256_" functions which are deprecated in OpenSSL 3.0)
    EVP_MD_CTX * context = EVP_MD_CTX_new();
    if(context == nullptr) { neroshop::print("EVP_MD_CTX_new failed", 1); return false; }
    // initialize
    if(EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(context);
        neroshop::print(ERR_error_string(ERR_get_error(), nullptr), 1);
        return false;
    }
    // update (generate hash)
    if(EVP_DigestUpdate(context, email.c_str(), email.length()) != 1) {
        EVP_MD_CTX_free(context);
        neroshop::print(ERR_error_string(ERR_get_error(), nullptr), 1);
        return false;
    }
    // finalize
    unsigned char sha256_hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    if(EVP_DigestFinal_ex(context, sha256_hash, &hash_len) != 1) {
        EVP_MD_CTX_free(context);
        neroshop::print(ERR_error_string(ERR_get_error(), nullptr), 1);
        return false;
    }
    // store hash in string
    std::stringstream ss;
    for(unsigned int i = 0; i < hash_len; ++i) 
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)sha256_hash[i];
    hash_out = ss.str();
#ifdef NEROSHOP_DEBUG0
    if(!email.empty()) neroshop::print("generated sha256 hash: " + hash_out + " (" + email + ")");
#endif    
    // if everything went well, free the context and return true
    EVP_MD_CTX_free(context); // renamed from "EVP_MD_CTX_destroy" in 1.1.0, same with EVP_MD_CTX_create => EVP_MD_CTX_new
    return true;
}
////////////////////
////////////////////
////////////////////
////////////////////
