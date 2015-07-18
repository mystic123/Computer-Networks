/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#ifndef	TCPCONNECTION_H
#define TCPCONNECTION_H

#include "utils.h"
#include "datacontainer.h"

class TCPConnection : public boost::enable_shared_from_this<TCPConnection>
{
public:
	typedef boost::shared_ptr<TCPConnection> ptr;

	static ptr create(boost::asio::io_service& io_srv, float_t time);

	boost::asio::ip::tcp::socket& socket();

	void start();

private:
	TCPConnection(boost::asio::io_service& io_srv, float_t time);

	void read_more();
	void send_data();
	std::string prepare_msg(const std::vector<std::pair<DataContainer::ip_addr, std::vector<double_t>>>& data);
	void refresh_ui(const boost::system::error_code& error);

	void handle_write(const boost::system::error_code& error,
			size_t bytes_transferred);

	void handle_read(const boost::system::error_code& error,
			size_t bytes_transferred);

	const float_t UI_REFRESH_TIME;
	uint32_t visible_line = 0;
	boost::asio::ip::tcp::socket socket_;
	boost::asio::deadline_timer timer_;
	boost::array<char, 1> buffer_;
};

#endif //TCPCONNECTION.H
