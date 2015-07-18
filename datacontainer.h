/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#ifndef DATACONTAINER_H
#define DATACONTAINER_H

#include "utils.h"

class DataContainer
{
public:
	typedef std::vector<uint64_t> times;
	typedef std::vector<std::vector<uint64_t>> results;
	typedef std::pair<uint64_t, results> entry;
	typedef boost::asio::ip::address_v4 ip_addr;

	static DataContainer* dc();

	void insertIP(ip_addr ip, uint32_t ttl);
	void insertDomainName(ip_addr ip, std::string name);
	void insertMeasure(ip_addr ip, measure_type type, uint32_t val);

	void setSSHRecord(std::string, bool val);

	void clearData();

	std::string domainName(ip_addr ip);
	bool hasSSH(std::string);

	std::map<ip_addr, entry> getDataCopy();

	void set_timer(boost::asio::deadline_timer* timer);

private:
	DataContainer() {};
	DataContainer(const DataContainer&);
	void operator=(const DataContainer&);

	void refresh_ttl(const boost::system::error_code& error);
	static DataContainer* dc_;

	std::map<ip_addr, entry> data_;
	std::map<ip_addr, std::string> domain_names_;
	std::map<std::string, bool> ssh_;
	boost::mutex mtx_;

	boost::asio::deadline_timer* timer_ = nullptr;
};

#endif //DATACONTAINER.H
