/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#ifndef	SERVERDNS_H
#define SERVERDNS_H

#include "utils.h"
#include "dns_question.h"

class ServerDNS
{
public:
	ServerDNS(boost::asio::io_service& io_service, uint32_t broadcast_time, bool ssh);

	void handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd);

	void handle_send_to(const boost::system::error_code& err, size_t bytes_transferred);

private:
	enum services {DELAY, SSH};
	void init_my_ip(boost::asio::io_service& io_service);

	void init_host_name();

	void handle_name_recv(const boost::system::error_code& error, size_t bytes_recvd);
	void handle_name_timeout(const boost::system::error_code& error);

	void send_a_query(const std::string& data, bool unicast = false);
	void send_a_response(dns_question q);

	void send_ptr_query(const std::string& data, bool unicast = false);
	void send_ptr_response(dns_question q, services serv);

	void broadcast(const boost::system::error_code& error);

	void search_for_hosts();

	void send_ssh();

	std::string host_name_;

	boost::asio::io_service& io_srv_;
	boost::asio::ip::udp::socket socket_;
	boost::asio::ip::udp::endpoint sender_endpoint_;
	boost::asio::ip::address_v4 my_ip_;
	boost::asio::ip::udp::endpoint multicast_endpoint_;
	boost::asio::deadline_timer timer_;
	boost::asio::deadline_timer name_timer_;
	enum { max_length = 1024 };
	char data_[max_length];

	uint32_t broadcast_time_;
	const uint32_t timeout_time_ = 5; /* time used when resolving host name conflicts */

	const bool ssh_avail_;

	static const std::string SSH_SERVICE;
	static const std::string DELAY_SERVICE;
	static const std::string LISTEN_ADDR;
	static const std::string MULTICAST_ADDR;
};

#endif //SERVERDNS.H
