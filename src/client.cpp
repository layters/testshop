#include "client.hpp"

neroshop::Client::Client() {//: socket(0) {
    create();
}
////////////////////
neroshop::Client::~Client() {
	close();
}
////////////////////
std::unique_ptr<neroshop::Client> neroshop::Client::singleton (nullptr);
////////////////////
void neroshop::Client::create() {
}
////////////////////
bool neroshop::Client::connect(unsigned int port, std::string address)
{
    return false;
}
////////////////////
void neroshop::Client::write(const std::string& text)
{
}
std::string neroshop::Client::read()
{
    return "";
}
////////////////////	
void neroshop::Client::close() {
}
////////////////////
void neroshop::Client::shutdown() {
}
////////////////////
////////////////////
void neroshop::Client::disconnect() { // if only shutdown() is called, the client socket will still be alive which is why we must call close() as well
	shutdown();
	close();
}
////////////////////
bool neroshop::Client::reconnect(unsigned int port, std::string address) { // kill socket first before attempting to re-connect
    close();
    return connect(port, address);
}
////////////////////
neroshop::Client * neroshop::Client::get_main_client() {
    if(!singleton.get()) { 
        singleton = std::make_unique<Client>();
    }
    return singleton.get();
}
////////////////////
////////////////////
bool neroshop::Client::is_connected() const { // https://stackoverflow.com/a/4142038 // can only work when close() is called
    return true;
}
////////////////////
////////////////////
////////////////////
