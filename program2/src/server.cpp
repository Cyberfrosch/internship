#include "server.h"

Server::Server() : acceptor(io_service, tcp::endpoint(tcp::v4(), 8888))
{
	
}

void Server::run()
{
	acceptor.listen();
	std::cout << "The server is running and waiting for connection\n";

	while (true)
	{
		tcp::socket socket(io_service);

		try
		{
			acceptor.accept(socket);
			std::cout << "Client connected\n";
		}
		catch (boost::system::system_error& err)
		{
			std::cout << "Error accepting client: " << err.what() << std::endl;
			continue;
		}

		boost::system::error_code error;
		try
		{
			while (error != boost::asio::error::eof)
			{
				boost::asio::read_until(socket, buf, '\n', error);

				if (error)
					throw boost::system::system_error(error);

				std::istream input_stream(&buf);
				std::string s;
				std::getline(input_stream, s);
				std::cout << (s.length() > 2 && std::stoi(s) % 32 == 0 ? "Data received: " + s : "Error: invalid data") << std::endl;

				buf.consume(buf.size());
			}
		}
		catch (boost::system::system_error& err)
		{
			std::cout << "Client disconnected\n";
			socket.close();
		}
	}
}
