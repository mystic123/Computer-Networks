/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#include "serverdns.h"
#include "dns_header.h"
#include "dns_question.h"
#include "dns_response.h"
#include "datacontainer.h"

#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const std::string ServerDNS::SSH_SERVICE = "_ssh._tcp.local.";
const std::string ServerDNS::DELAY_SERVICE = "_opoznienia._udp.local.";
const std::string ServerDNS::LISTEN_ADDR = "0.0.0.0";
const std::string ServerDNS::MULTICAST_ADDR = "224.0.0.251";

ServerDNS::ServerDNS(boost::asio::io_service& io_service, uint32_t broadcast_time, bool ssh): io_srv_(io_service), socket_(io_service),
		multicast_endpoint_(boost::asio::ip::address::from_string(MULTICAST_ADDR), MULTICAST_PORT), timer_(io_srv_),
		name_timer_(io_srv_), broadcast_time_(broadcast_time), ssh_avail_(ssh)
{
	init_my_ip(io_srv_);

	// Create the socket so that multiple may be bound to the same address.
	boost::asio::ip::udp::endpoint listen_endpoint(boost::asio::ip::address::from_string(LISTEN_ADDR), MULTICAST_PORT);
	socket_.open(listen_endpoint.protocol());
	socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	socket_.bind(listen_endpoint);

	// Join the multicast group.
	socket_.set_option(boost::asio::ip::multicast::join_group(boost::asio::ip::address::from_string(MULTICAST_ADDR)));
	socket_.set_option(boost::asio::ip::multicast::enable_loopback(false));

	init_host_name();
}

void ServerDNS::handle_name_recv(const boost::system::error_code& error, size_t bytes_recvd)
{
	if (!error)
	{
		dns_header hdr = dns_header::fromString(data_);
		if (hdr.qr() == dns_header::response) {
			dns_response r = dns_response::fromString(data_+dns_header::LENGTH/*+size*/);
			if (r.name().find(DELAY_SERVICE) != std::string::npos) {
				if (r.type() == dns_response::A) {
					if (r.name().find(host_name_) !=std::string::npos) {
						/* conflict */
						if (host_name_[host_name_.size()-2] == '_') {
							host_name_[host_name_.size()-1] = host_name_[host_name_.size()-1]+1;
						}
						else {
							host_name_ += "_0";
						}
						name_timer_.expires_from_now(boost::posix_time::seconds(timeout_time_));
					}
				}
			}
		}
		socket_.async_receive_from(
				boost::asio::buffer(data_, max_length), sender_endpoint_,
				boost::bind(&ServerDNS::handle_name_recv, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
	}
}

void ServerDNS::handle_name_timeout(const boost::system::error_code& error)
{
	if (!error) {
		/* no one has same name, we can start working */
		socket_.cancel();
		socket_.async_receive_from(
				boost::asio::buffer(data_, max_length), sender_endpoint_,
				boost::bind(&ServerDNS::handle_receive_from, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
		timer_.expires_from_now(boost::posix_time::seconds(broadcast_time_));
		timer_.async_wait(boost::bind(&ServerDNS::broadcast, this, boost::asio::placeholders::error));
	}
	else {
		name_timer_.async_wait(boost::bind(&ServerDNS::handle_name_timeout, this,
				boost::asio::placeholders::error));
	}
}

void ServerDNS::handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd)
{
	if (!error)
	{
		dns_header hdr = dns_header::fromString(data_);
		if (hdr.qr() == dns_header::query_type) {
			/* received query */
			dns_question q = dns_question::fromString(data_+dns_header::LENGTH);
			//q.print();
			if (q.type() == dns_question::PTR) {
				if (q.addr().compare(DELAY_SERVICE) == 0) {
					/* asking for delay service */
					send_ptr_response(q, DELAY);
				}
				else if (ssh_avail_ && q.addr().compare(SSH_SERVICE) == 0) {
					/* asking for ssh service */
					send_ptr_response(q, SSH);
				}
			}
			else if (q.type() == dns_question::A) {
				if (q.addr().compare(host_name_+"."+DELAY_SERVICE) == 0) {
					/* asking for my ip */
					send_a_response(q);
				}
			}
		}
		else if (hdr.qr() == dns_header::response) {
			/* received response */
			int i = 0;
			size_t size = 0;
			/* omitting questions if response contains any */
			while (i < hdr.qdcount()) {
				dns_question q = dns_question::fromString(data_+dns_header::LENGTH);
				size += q.size();
				i++;
			}
			dns_response r = dns_response::fromString(data_+dns_header::LENGTH+size);
			if (r.name().find(DELAY_SERVICE) != std::string::npos || r.name().find(SSH_SERVICE) != std::string::npos) {
				/* reponse for delay or ssh serivce */
				if (r.name().find(DELAY_SERVICE) != std::string::npos) {
					if (r.type() == dns_response::PTR) {
						/* asking for ip */
						send_a_query(r.rdata());
					}
					else if (r.type() == dns_response::A) {
						/* saving in known hosts database */
						uint32_t ip = 0;
						for (size_t i = 0; i < 4; i++) {
							ip += (unsigned char)(r.rdata()[i]);
							if (i < 3)
								ip = ip << 8;
						}
						std::string ip_s = "";
						for (int i = 0; i < 4; i++) {
							ip_s = std::to_string((ip%256)) + ip_s;
							if (i < 3) {
								ip_s = "." + ip_s;
								ip = ip >> 8;
							}
						}
						DataContainer::dc()->insertIP(boost::asio::ip::address_v4::from_string(ip_s), r.ttl());
						DataContainer::dc()->insertDomainName(boost::asio::ip::address_v4::from_string(ip_s), r.name().substr(0, r.name().find(DELAY_SERVICE)-1));
					}
				}
				else if (r.name().find(SSH_SERVICE) != std::string::npos) {
					if (r.type() == dns_response::PTR) {
						DataContainer::dc()->setSSHRecord(r.rdata().substr(0, r.rdata().find(SSH_SERVICE)-1), true);
					}
				}
			}
		}


		socket_.async_receive_from(
				boost::asio::buffer(data_, max_length), sender_endpoint_,
				boost::bind(&ServerDNS::handle_receive_from, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
	}
}

void ServerDNS::handle_send_to(const boost::system::error_code& err, size_t bytes_transferred)
{
}

void ServerDNS::init_my_ip(boost::asio::io_service& io_service)
{
	//solution for obtaining own IP address form stackoverflow
	struct ifaddrs * ifAddrStruct=NULL;
	struct ifaddrs * ifa=NULL;
	void * tmpAddrPtr=NULL;
	getifaddrs(&ifAddrStruct);
	std::string addressBuffer2;
	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr) {
			continue;
		}
		if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
			// is a valid IP4 Address
			tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			std::string str(addressBuffer);
			if ((str.find("127.0") == std::string::npos) &&
					(str.find("::") == std::string::npos))
				addressBuffer2 = str;
		} else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
			// is a valid IP6 Address
			tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
			std::string str(addressBuffer);
			if ((str.find("127.0") == std::string::npos) &&
					(str.find("::") == std::string::npos))
				addressBuffer2 = str;
		}
	}
	my_ip_ = boost::asio::ip::address::from_string(addressBuffer2).to_v4();
	if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
}

void ServerDNS::init_host_name()
{
	host_name_ = boost::asio::ip::host_name();
	size_t pos = host_name_.find(".local");
	if (pos != std::string::npos) {
		host_name_ = host_name_.substr(0, pos);
	}

	send_a_query(host_name_+"."+DELAY_SERVICE);

	name_timer_.expires_from_now(boost::posix_time::seconds(timeout_time_));
	name_timer_.async_wait(boost::bind(&ServerDNS::handle_name_timeout, this, boost::asio::placeholders::error));
	socket_.async_receive_from(
			boost::asio::buffer(data_, max_length), sender_endpoint_,
			boost::bind(&ServerDNS::handle_name_recv, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}

void ServerDNS::send_a_query(const std::string& data, bool unicast)
{
	dns_header hdr;
	hdr.setQr(dns_header::query_type);
	hdr.setQdcount(1);
	boost::shared_ptr<boost::asio::streambuf> req_buf(new boost::asio::streambuf());
	std::ostream os(&*req_buf);
	os << hdr;
	dns_question q(data, dns_question::A);
	if (unicast) {
		q.setUnicastResponse(true);
	}
	os << q;

	socket_.async_send_to(
			boost::asio::buffer(req_buf->data()), multicast_endpoint_,
			boost::bind(&ServerDNS::handle_send_to, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}

void ServerDNS::send_a_response(dns_question q)
{
	dns_header hdr;
	hdr.setQr(dns_header::response);
	hdr.setAncount(1);
	boost::shared_ptr<boost::asio::streambuf> req_buf(new boost::asio::streambuf());
	std::ostream os(&*req_buf);
	os << hdr;
	std::string s(host_name_);
	dns_response r;
	r.setType(dns_response::A);
	r.setName(q.addr());
	std::string ip(my_ip_.to_string());
	std::vector<std::string> strs;
	boost::split(strs, ip, boost::is_any_of("."));
	std::string tmp(4,0);
	size_t i = 0;
	for (std::string s: strs) {
		int r = stoi(s);
		tmp[i] = (char)(r & 0xFF);
		i++;
	}
	r.setRdata(tmp.c_str());
	os << r;
	boost::asio::ip::udp::endpoint endpoint_(boost::asio::ip::address::from_string(MULTICAST_ADDR), MULTICAST_PORT);
	socket_.async_send_to(req_buf->data(), endpoint_,
			boost::bind(&ServerDNS::handle_send_to, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}

void ServerDNS::send_ptr_query(const std::string& data, bool unicast)
{
	dns_header hdr;
	hdr.setQr(dns_header::query_type);
	hdr.setQdcount(1);
	boost::shared_ptr<boost::asio::streambuf> req_buf(new boost::asio::streambuf());
	std::ostream os(&*req_buf);
	os << hdr;
	dns_question q(data, dns_question::PTR);
	if (unicast) {
		q.setUnicastResponse(true);
	}
	os << q;

	socket_.async_send_to(
			boost::asio::buffer(req_buf->data()), multicast_endpoint_,
			boost::bind(&ServerDNS::handle_send_to, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}

void ServerDNS::send_ptr_response(dns_question q, services serv)
{
	dns_header hdr;
	hdr.setQr(dns_header::response);
	hdr.setAncount(1);
	boost::shared_ptr<boost::asio::streambuf> req_buf(new boost::asio::streambuf());
	std::ostream os(&*req_buf);
	os << hdr;
	dns_response r;
	r.setType(dns_response::PTR);
	r.setName(q.addr());
	if (serv == DELAY) {
		r.setRdata(host_name_ + "." + DELAY_SERVICE);
	}
	else if (serv == SSH) {
		r.setRdata(host_name_ + "." + SSH_SERVICE);
	}
	os << r;
	boost::asio::ip::udp::endpoint endpoint_(boost::asio::ip::address::from_string(MULTICAST_ADDR), MULTICAST_PORT);
	socket_.async_send_to(req_buf->data(), endpoint_,
			boost::bind(&ServerDNS::handle_send_to, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}

void ServerDNS::broadcast(const boost::system::error_code& error)
{
	if (!error) {
		search_for_hosts();
		send_ssh();
		timer_.expires_from_now(boost::posix_time::seconds(broadcast_time_));
		timer_.async_wait(boost::bind(&ServerDNS::broadcast, this,
				boost::asio::placeholders::error));
	}
}

void ServerDNS::search_for_hosts()
{
	send_ptr_query(DELAY_SERVICE);
}

void ServerDNS::send_ssh()
{
	send_ptr_query(SSH_SERVICE);
}
