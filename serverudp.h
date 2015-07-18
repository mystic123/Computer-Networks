/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#ifndef	SERVERUDP_H
#define SERVERUDP_H

#include "utils.h"

class ServerUDP
{
public:
	ServerUDP(boost::asio::io_service& io_srv, int port);

	void run_server();

private:
	void start_receive();

	const int port_;
	boost::asio::io_service& io_srv_;
	boost::asio::ip::udp::socket socket_;

};

#endif //SERVERUDP.H
