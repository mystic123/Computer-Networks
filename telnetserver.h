/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#ifndef	TELNETSERVER_H
#define TELNETSERVER_H

#include "tcpconnection.h"

class TelnetServer
{
public:
	TelnetServer(boost::asio::io_service& io_srv, uint16_t port, float_t time);

private:
	void start_accept();

	void handle_accept(TCPConnection::ptr new_connection, const boost::system::error_code& error);

	boost::asio::ip::tcp::acceptor acceptor_;
	const float_t UI_REFRESH_TIME;
};

#endif //TELNETSERVER.H
