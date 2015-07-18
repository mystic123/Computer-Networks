/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#ifndef CLIENTICMP_H
#define CLIENTICMP_H

#include "utils.h"

class ClientICMP
{
public:
	ClientICMP(boost::asio::io_service& io_srv, std::string dest);

	void measure_latency();

	bool finished();

private:
	void start_send();

	void start_receive();

	void handle_send(const boost::system::error_code& error, size_t bytes);
	void handle_timeout(const boost::system::error_code& error);
	void handle_receive(size_t length);

	static std::string initialize_body();

	bool finished_ = false;

	boost::mutex mtx_;
	boost::asio::ip::icmp::resolver resolver_;
	boost::asio::ip::icmp::endpoint destination_;
	boost::asio::ip::icmp::socket socket_;
	boost::asio::deadline_timer timer_;
	uint64_t time_sent_ = 0;
	boost::asio::streambuf reply_buffer_;
	std::string dest_;

	static const std::string BODY; //ICMP body
};

#endif //CLIENTICMP.H
