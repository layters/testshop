#include "server.hpp"

neroshop::Server::Server()
{}
////////////////////
neroshop::Server::~Server() {
	close();
}
////////////////////
bool neroshop::Server::bind(unsigned int port)
{
	return true;
}
////////////////////	
bool neroshop::Server::listen()
{
	return true;
}
////////////////////
bool neroshop::Server::accept()
{
	return true;
}
////////////////////
bool neroshop::Server::accept_all()
{
	return true;
}
////////////////////
void neroshop::Server::write(const std::string& message) {
}
////////////////////
std::string neroshop::Server::read() // receive data
{
	return "";
}
////////////////////
void neroshop::Server::close() {
}
////////////////////
void neroshop::Server::shutdown() {
}
////////////////////
////////////////////
////////////////////
//template<typename F> void neroshop::Server::bind(const std::string& name, F func) {//F response) {
	// if client requests a string e.g "LOGIN" then server will respond with login()
	//response();
	// I don't know what I'm doing here ...
	//std::map<std::string, F> rr_pair = { { request, response }, };
//}
////////////////////
