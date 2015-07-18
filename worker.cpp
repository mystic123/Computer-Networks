/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#include "worker.h"
#include "datacontainer.h"

Worker::ptr Worker::create(boost::asio::io_service& io_srv, boost::asio::ip::address_v4 ip)
{
	return ptr(new Worker(io_srv, ip));
}

Worker::Worker(boost::asio::io_service& io_srv, boost::asio::ip::address_v4 ip): io_srv_(io_srv), ip_(ip),
	cudp(io_srv_, ip_.to_string()), cicmp(io_srv_, ip_.to_string()), ctcp(io_srv, ip.to_string())
{
}

void Worker::work()
{
	cicmp.measure_latency();
	cudp.measure_latency();

	if (DataContainer::dc()->hasSSH(DataContainer::dc()->domainName(ip_))) {
		ctcp.measure_latency();
	}

}

bool Worker::finished()
{
	bool tcp = true;
	if (DataContainer::dc()->hasSSH(DataContainer::dc()->domainName(ip_))) {
			tcp = ctcp.finished();
	}
	return cudp.finished() && cicmp.finished() && tcp;
}
