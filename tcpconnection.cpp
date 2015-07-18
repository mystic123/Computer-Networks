/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#include "tcpconnection.h"
#include "datacontainer.h"
#include <iterator>
#include <vector>
#include <algorithm>

const uint32_t WIDTH = 80;
const uint32_t HEIGHT = 24;

const char ESC = 0x1B;
const char * CLEAR = "[2J";
const char * HOME = "[H";

struct compare {
	bool operator() (const std::pair<DataContainer::ip_addr, std::vector<double_t>>& a,
			const std::pair<DataContainer::ip_addr, std::vector<double_t>>& b)
	{
		double_t a_v, b_v;
		a_v = b_v = 0;
		for (size_t i = 0; i < 3; i++) {
			a_v += a.second[i];
			b_v += b.second[i];
		}
		return a_v > b_v;
	}
} comp;

TCPConnection::ptr TCPConnection::create(boost::asio::io_service& io_srv, float_t time)
{
	return ptr(new TCPConnection(io_srv, time));
}

boost::asio::ip::tcp::socket& TCPConnection::socket()
{
	return socket_;
}

void TCPConnection::start()
{
	send_data();
	boost::asio::async_read(socket_, boost::asio::buffer(buffer_),
			boost::bind(&TCPConnection::handle_read, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
	timer_.expires_from_now(boost::posix_time::duration_from_string("00:00:"+std::to_string(UI_REFRESH_TIME)));
	timer_.async_wait(boost::bind(&TCPConnection::refresh_ui, this, boost::asio::placeholders::error));
}

void TCPConnection::send_data()
{
	boost::shared_ptr<std::string> msg(new std::string);
	auto data = DataContainer::dc()->getDataCopy();
	auto it = data.begin();
	if (data.size() > HEIGHT) {
		if (visible_line + HEIGHT -1 <= data.size()) {
			std::advance(it, visible_line);
		}
		else if (data.size() - HEIGHT - 1 > 0) {
			std::advance(it, data.size() - HEIGHT - 1);
			visible_line = HEIGHT + 1;
		}
	}
	std::vector<std::pair<DataContainer::ip_addr, std::vector<double_t>>> send_data;
	for (size_t i = 0; i < HEIGHT - 1 && it != data.end(); i++) {
		DataContainer::ip_addr ip = it->first;
		std::vector<std::vector<uint64_t>> res;
		res = it->second.second;
		std::vector<double_t> avg_res(3);
		for (int i = 0; i < 3; i++) {
			uint64_t sum = 0;
			for (uint64_t x: res[i]) {
				sum+=x;
			}
			double_t avg = 0;
			if (res[i].size() > 0) {
				avg = (double_t)sum/(double_t)res[i].size();
			}
			else {
				avg = 10*MICRO_MULT;
			}
			avg_res[i] = avg/1000.0;
		}
		send_data.push_back(std::make_pair(ip, avg_res));
		it++;
	}
	std::sort(send_data.begin(), send_data.end(), comp);
	(*msg += ESC) += CLEAR;
	(*msg += ESC) += HOME;
	*msg += prepare_msg(send_data);
	boost::asio::async_write(socket_, boost::asio::buffer(*msg),
			boost::bind(&TCPConnection::handle_write, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}

TCPConnection::TCPConnection(boost::asio::io_service& io_srv, float_t time): UI_REFRESH_TIME(time), socket_(io_srv), timer_(io_srv)
{
}

void TCPConnection::read_more()
{
	boost::asio::async_read(socket_, boost::asio::buffer(buffer_),
			boost::bind(&TCPConnection::handle_read, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}

void TCPConnection::handle_write(const boost::system::error_code& error,
		size_t bytes_transferred)
{
}

void TCPConnection::handle_read(const boost::system::error_code& error,
		size_t bytes_transferred)
{
	read_more();
}

std::string TCPConnection::prepare_msg(
		const std::vector<
		std::pair<DataContainer::ip_addr, std::vector<double_t> > >& data)
{
	std::string ret;
	auto it = data.begin();
	double_t max_delay = 0;
	if (it != data.end()) {
		max_delay = (it->second[0] + it->second[1] + it->second[2])/3;
		for (auto it: data) {
			ret += it.first.to_string();
			double_t avg_delay = (it.second[0] + it.second[1] + it.second[2])/3;
			std::string numbers;
			numbers += std::to_string(it.second[0]);
			numbers += " ";
			if (it.second[1] < 10000.0) {
				numbers += std::to_string(it.second[1]);
			}
			else {
				numbers += "NaN";
			}
			numbers += " ";
			numbers += std::to_string(it.second[2]);
			numbers += "\n";
			uint32_t spaces = ((WIDTH - it.first.to_string().size() + numbers.size()-1)*avg_delay)/max_delay;
			ret += " ";
			for (uint32_t i = 0; i < spaces-1; i++) {
				ret += " ";
			}
			ret += numbers;
		}
	}
	return ret;
}

void TCPConnection::refresh_ui(const boost::system::error_code& error)
{
	if (!error) {
		send_data();
		timer_.expires_from_now(boost::posix_time::duration_from_string("00:00:"+std::to_string(UI_REFRESH_TIME)));
		timer_.async_wait(boost::bind(&TCPConnection::refresh_ui, this, boost::asio::placeholders::error));
	}
}
