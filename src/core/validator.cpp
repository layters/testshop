#include "validator.hpp"

#include "database.hpp"
#include "config.hpp"
#include "util.hpp"

#include <regex>
#include <cctype> // isspace, isalpha, isalnum, etc.
#include <iomanip> // std::setfill, std::setw

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
    if(neroshop::string::lower(username) == "guest") {
        neroshop::print("The name \"Guest\" is reserved for guests ONLY");
        return false;
    }
    //-------------------------------------------------------////////////
    // postgresql
    //-------------------------------------------------------////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    // check db to see if username is not already taken (last thing to worry about)
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
	if(DB::Postgres::get_singleton()->table_exists("users")) 
	{   // make sure table Users exists first
	    std::string name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM users WHERE name = $1;", { neroshop::string::lower(username) });//+ DB::Postgres::to_psql_string(neroshop::string::lower(username)));
	    if(name == neroshop::string::lower(username)) { // names are stored in lowercase
	        neroshop::print("This username is already taken", 2);
	        /*DB::Postgres::get_singleton()->finish();*/
	        return false;
	    }//std::cout << "name: " << name << " (retrieved from db)" << std::endl;
	}    
	// make sure any deleted user's name is not re-used
	if(DB::Postgres::get_singleton()->table_exists("deleted_users")) {
	    std::string deleted_user = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM deleted_users WHERE name = $1;", { username });//+ DB::Postgres::to_psql_string(username));
	    if(deleted_user == neroshop::string::lower(username)) {
	        neroshop::print("This username cannot be used"/*re-used"*/, 2);
            /*DB::Postgres::get_singleton()->finish();*/
            return false;
	    }
	}    
    /*DB::Postgres::get_singleton()->finish();*/
    //-------------------------------------------------------////////////    
    return true; // default return value
#endif
    return false;    
}
//-------------------------------------------------------
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
//-------------------------------------------------------
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
    //-------------------------------------------------------////////////
    // postgresql
    //-------------------------------------------------------////////////
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
    //-------------------------------------------------------////////////
    return true;
#endif
    return false;    
}
//-------------------------------------------------------
//-------------------------------------------------------
bool neroshop::Validator::validate_sha256_hash(const std::string& email, const std::string& hash) { // raw/unsalted hash
    std::string temp_hash;
    if(!generate_sha256_hash(email, temp_hash)) return false;
    return (temp_hash == hash);
}
//-------------------------------------------------------
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
//-------------------------------------------------------
//-------------------------------------------------------
