/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#ifndef CLIENTTCP_H
#define CLIENTTCP_H

#include "utils.h"

class ClientTCP
{
public:
	ClientTCP(boost::asio::io_service& io_srv, std::string host);

	void measure_latency();

	bool finished();

private:
	void handle_connect(const boost::system::error_code& error);

	void handle_timeout(const boost::system::error_code& error);

	bool finished_ = false;
	boost::mutex mtx_;
	boost::asio::ip::tcp::socket socket_;
	std::string host_;
	boost::asio::ip::tcp::endpoint endpoint_;

	boost::asio::deadline_timer timer_;

	uint64_t time_before_ = 0;
	static const uint16_t SSH_PORT = 22;
};

#endif //CLIENTTCP.H
