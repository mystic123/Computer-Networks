#include "utils.h"
/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#include "datacontainer.h"

DataContainer* DataContainer::dc_ = nullptr;

DataContainer* DataContainer::dc()
{
	if (!dc_) {
		dc_ = new DataContainer();
	}
	return dc_;
}

void DataContainer::insertIP(ip_addr ip, uint32_t ttl)
{
	mtx_.lock();
	auto it = data_.find(ip);
	if (it != data_.end()) {
		it->second.first = ttl;
	}
	else {
		results r;
		times t;
		r.push_back(t);
		r.push_back(t);
		r.push_back(t);
		entry e(ttl, r);
		data_.insert(make_pair(ip, e));
	}
	mtx_.unlock();
}

void DataContainer::insertDomainName(ip_addr ip, std::string name)
{
	mtx_.lock();
	domain_names_.insert(make_pair(ip, name));
	mtx_.unlock();
}

void DataContainer::setSSHRecord(std::string name, bool val)
{
	mtx_.lock();
	auto it = ssh_.find(name);
	if (it != ssh_.end()) {
		it->second = val;
	}
	else {
		ssh_.insert(make_pair(name, val));
	}
	mtx_.unlock();
}

void DataContainer::clearData()
{
	mtx_.lock();
	ssh_.clear();
	domain_names_.clear();
	data_.clear();
	mtx_.unlock();
}

std::string DataContainer::domainName(ip_addr ip)
{
	mtx_.lock();
	auto it = domain_names_.find(ip);
	std::string r = "";
	if (it != domain_names_.end()) {
		r = it->second;
	}
	mtx_.unlock();
	return r;
}

bool DataContainer::hasSSH(std::string name)
{
	mtx_.lock();
	auto it = ssh_.find(name);
	bool r = false;
	if (it != ssh_.end()) {
		r = it->second;
	}
	mtx_.unlock();
	return r;
}

void DataContainer::insertMeasure(ip_addr ip, measure_type type, uint32_t val)
{
	mtx_.lock();
	auto it = data_.find(ip);
	if (it != data_.end()) {
		if (it->second.second[type].size() == 10) {
			it->second.second[type].erase(it->second.second[type].begin());
		}
		it->second.second[type].push_back(val);
		if (type == measure_type::UDP) {
		}
		else if (type == measure_type::TCP) {
		}
		else if (type == measure_type::ICMP) {
		}
	}
	mtx_.unlock();
}

std::map<DataContainer::ip_addr, DataContainer::entry> DataContainer::getDataCopy()
{
	mtx_.lock();
	auto d = data_;
	mtx_.unlock();
	return d;
}

void DataContainer::set_timer(boost::asio::deadline_timer* timer)
{
	timer_ = timer;
	timer_->expires_from_now(boost::posix_time::seconds(1));
	timer_->async_wait(boost::bind(&DataContainer::refresh_ttl, this, boost::asio::placeholders::error));
}

void DataContainer::refresh_ttl(const boost::system::error_code& error)
{
	if (!error) {
		mtx_.lock();
		for (auto it = data_.begin(); it != data_.end(); it++) {
			it->second.first--;
		}
		mtx_.unlock();
		timer_->expires_from_now(boost::posix_time::seconds(1));
		timer_->async_wait(boost::bind(&DataContainer::refresh_ttl, this, boost::asio::placeholders::error));
	}
}
