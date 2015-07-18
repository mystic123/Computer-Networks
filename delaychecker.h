/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#ifndef DELAYCHECKER_H_
#define DELAYCHECKER_H_

#include "utils.h"
#include "worker.h"

class DelayChecker {
public:
	DelayChecker(boost::asio::io_service& io_srv, uint32_t time = 1);

private:
	void check(const boost::system::error_code& error);
	void clear(const boost::system::error_code& error);

	boost::asio::io_service& io_srv_;
	uint32_t time_;
	boost::asio::deadline_timer timer_;
	boost::asio::deadline_timer clear_timer_;

	boost::mutex mtx_;
	std::vector<Worker::ptr> workers_;
};

#endif /* DELAYCHECKER_H_ */
