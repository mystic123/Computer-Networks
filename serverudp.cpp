/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#include "serverudp.h"
#include "endian.h"

ServerUDP::ServerUDP(boost::asio::io_service& io_srv, int port) : port_(port), io_srv_(io_srv),
socket_(io_srv_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port_))
{
}

void ServerUDP::run_server()
{
	io_srv_.run();
	start_receive();
}

void ServerUDP::start_receive()
{
	while (true) {
		boost::asio::ip::udp::endpoint remote_endpoint;
		boost::array<uint64_t, 1> recv_buffer;
		boost::array<uint64_t, 2> send_buffer;
		socket_.receive_from(boost::asio::buffer(recv_buffer), remote_endpoint);

		send_buffer[0] = recv_buffer[0];
		send_buffer[1] = htobe64(GetTimeStamp());

		socket_.send_to(boost::asio::buffer(send_buffer), remote_endpoint);
	}
}
