/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#include "clienttcp.h"
#include "datacontainer.h"

ClientTCP::ClientTCP(boost::asio::io_service& io_srv, std::string host): socket_(io_srv), host_(host),
endpoint_(boost::asio::ip::address::from_string(host_), SSH_PORT), timer_(io_srv)
{
}

void ClientTCP::measure_latency()
{
	time_before_ = GetTimeStamp();
	timer_.expires_from_now(boost::posix_time::seconds(MAX_DELAY));
	timer_.async_wait(boost::bind(&ClientTCP::handle_timeout, this, boost::asio::placeholders::error));
	socket_.async_connect(endpoint_, boost::bind(&ClientTCP::handle_connect, this, boost::asio::placeholders::error));
}

void ClientTCP::handle_connect(const boost::system::error_code& error)
{
	uint64_t after = GetTimeStamp();
	DataContainer::dc()->insertMeasure(boost::asio::ip::address_v4::from_string(host_), measure_type::TCP, after-time_before_);
	mtx_.lock();
	finished_ = true;
	mtx_.unlock();
}

void ClientTCP::handle_timeout(const boost::system::error_code& error)
{
	if (!error) {
		DataContainer::dc()->insertMeasure(boost::asio::ip::address_v4::from_string(host_), measure_type::TCP, MAX_DELAY*MICRO_MULT);
		mtx_.lock();
		finished_ = true;
		mtx_.unlock();
	}
}

bool ClientTCP::finished()
{
	mtx_.lock();
	auto ret = finished_;
	mtx_.unlock();
	return ret;
}
