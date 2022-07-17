#include "../include/server.hpp"

neroshop::Server::Server()
{}
//////////
neroshop::Server::~Server() {
	close();
}
//////////
bool neroshop::Server::bind(unsigned int port)
{
	return true;
}
//////////	
bool neroshop::Server::listen()
{
	return true;
}
//////////
bool neroshop::Server::accept()
{
	return true;
}
//////////
bool neroshop::Server::accept_all()
{
	return true;
}
//////////
void neroshop::Server::write(const std::string& text) {
}
//////////
std::string neroshop::Server::read() // receive data
{
	return "";
}
//////////
void neroshop::Server::close() {
}
//////////
void neroshop::Server::shutdown() {
}
//////////
//////////
