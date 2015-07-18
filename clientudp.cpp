/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#include "clientudp.h"
#include "endian.h"
#include "datacontainer.h"

ClientUDP::ClientUDP(boost::asio::io_service& io_srv, std::string host): socket_(io_srv), timer_(io_srv)
{
	boost::asio::ip::udp::resolver resolver(io_srv);
	boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), host, "3382");
	endpoint_ = *resolver.resolve(query);
}

void ClientUDP::measure_latency()
{
	socket_.open(boost::asio::ip::udp::v4());

	time_ = GetTimeStamp();
	send_buf_ = {{ htobe64(time_) }};
	socket_.async_send_to(boost::asio::buffer(send_buf_), endpoint_,
			bind(&ClientUDP::handle_send, this, 
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));

	start_receive();
}

void ClientUDP::handle_timeout(const boost::system::error_code& error)
{
	if (!error) {
		DataContainer::dc()->insertMeasure(endpoint_.address().to_v4(), measure_type::UDP, MAX_DELAY*MICRO_MULT);
		mtx_.lock();
		finished_ = true;
		mtx_.unlock();
	}
}

void ClientUDP::handle_send(const boost::system::error_code& error, size_t bytes)
{
	timer_.expires_from_now(boost::posix_time::seconds(MAX_DELAY));
	timer_.async_wait(boost::bind(&ClientUDP::handle_timeout, this, boost::asio::placeholders::error));
}

void ClientUDP::start_receive()
{
	socket_.async_receive_from(boost::asio::buffer(recv_buf_), remote_endpoint_,
			bind(&ClientUDP::handle_receive, this, 
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}

void ClientUDP::handle_receive(const boost::system::error_code& error, size_t bytes)
{
	if (bytes == 2*sizeof(uint64_t)) {
		DataContainer::dc()->insertMeasure(endpoint_.address().to_v4(), measure_type::UDP, GetTimeStamp()-time_);
		mtx_.lock();
		finished_ = true;
		mtx_.unlock();
	}
}

bool ClientUDP::finished()
{
	mtx_.lock();
	auto ret = finished_;
	mtx_.unlock();
	return ret;
}
