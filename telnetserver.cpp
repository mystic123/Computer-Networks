/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#include "telnetserver.h"

TelnetServer::TelnetServer(boost::asio::io_service& io_srv, uint16_t port, float_t time) : acceptor_(io_srv,
		boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), UI_REFRESH_TIME(time)
{
	start_accept();
}

void TelnetServer::start_accept()
{
	TCPConnection::ptr new_connection = TCPConnection::create(acceptor_.get_io_service(), UI_REFRESH_TIME);
	acceptor_.async_accept(new_connection->socket(), boost::bind(&TelnetServer::handle_accept, this, new_connection,
			boost::asio::placeholders::error));
}

void TelnetServer::handle_accept(TCPConnection::ptr new_connection, const boost::system::error_code& error)
{
	if (!error) {
		new_connection->start();
	}

	start_accept();
}
