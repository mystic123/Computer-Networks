/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#include "delaychecker.h"
#include "datacontainer.h"

DelayChecker::DelayChecker(boost::asio::io_service& io_srv, uint32_t time): io_srv_(io_srv), time_(time),
timer_(io_srv, boost::posix_time::seconds(time)), clear_timer_(io_srv, boost::posix_time::seconds(MAX_DELAY+1)), workers_()
{
	timer_.async_wait(boost::bind(&DelayChecker::check, this, boost::asio::placeholders::error));
	clear_timer_.async_wait(boost::bind(&DelayChecker::clear, this, boost::asio::placeholders::error));
}

void DelayChecker::check(const boost::system::error_code& error)
{
	auto data_cpy = DataContainer::dc()->getDataCopy();
	mtx_.lock();
	for (auto it = data_cpy.begin(); it != data_cpy.end(); it++) {
		Worker::ptr ptr(Worker::create(io_srv_, it->first));
		workers_.push_back(ptr);
		ptr->work();
	}
	mtx_.unlock();
	timer_.expires_from_now(boost::posix_time::seconds(time_));
	timer_.async_wait(boost::bind(&DelayChecker::check, this, boost::asio::placeholders::error));
}

void DelayChecker::clear(const boost::system::error_code& error)
{
	if (!error) {
		mtx_.lock();
		auto it = workers_.begin();
		while (it != workers_.end()) {
			if (it->get()->finished()) {
				it = workers_.erase(it);
			}
			else {
				it++;
			}
		}
		mtx_.unlock();
		clear_timer_.expires_from_now(boost::posix_time::seconds(MAX_DELAY+1));
		clear_timer_.async_wait(boost::bind(&DelayChecker::clear, this, boost::asio::placeholders::error));
	}
}
