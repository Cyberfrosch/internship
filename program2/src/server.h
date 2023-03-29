#pragma once

#include <iostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Server
{
private:
	boost::asio::io_service io_service;
	boost::asio::streambuf buf;
	tcp::acceptor acceptor;
public:
	Server();
	void run();
};