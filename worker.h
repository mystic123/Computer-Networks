/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#ifndef WORKER_H_
#define WORKER_H_

#include "utils.h"
#include "clientudp.h"
#include "clienticmp.h"
#include "clienttcp.h"

class Worker: public boost::enable_shared_from_this<Worker>
{
public:
	typedef boost::shared_ptr<Worker> ptr;
	static ptr create(boost::asio::io_service& io_srv, boost::asio::ip::address_v4 ip);

	void work();
	bool finished();

private:
	Worker(boost::asio::io_service& io_srv, boost::asio::ip::address_v4);
	boost::asio::io_service& io_srv_;
	boost::asio::ip::address_v4 ip_;

	ClientUDP cudp;
	ClientICMP cicmp;
	ClientTCP ctcp;
};

#endif /* WORKER_H_ */
