/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#ifndef CLIENTUDP_H
#define CLIENTUDP_H

#include "utils.h"

class ClientUDP
{
public:
	ClientUDP(boost::asio::io_service& io_srv, std::string host);

	void measure_latency();

	bool finished();

private:
	void handle_send(const boost::system::error_code& error, size_t bytes);

	void start_receive();

	void handle_receive(const boost::system::error_code& error, size_t bytes);

	void handle_timeout(const boost::system::error_code& error);

	bool finished_ = false;
	boost::mutex mtx_;

	uint64_t time_ = 0;
	boost::asio::ip::udp::socket socket_;
	boost::asio::ip::udp::endpoint endpoint_;
	boost::asio::ip::udp::endpoint remote_endpoint_;
	boost::asio::deadline_timer timer_;
	boost::array<uint64_t, 1> send_buf_;
	boost::array<uint64_t, 2> recv_buf_;
};

#endif //CLIENTUDP.H
