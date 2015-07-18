/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#include <iostream>
#include <cstdlib>
#include <vector>
#include "utils.h"
#include "datacontainer.h"
#include "serverdns.h"
#include "serverudp.h"
#include "tcpconnection.h"
#include "telnetserver.h"
#include "delaychecker.h"

using namespace std;
namespace po = boost::program_options;

int main(int argc, char ** argv)
{
	uint16_t udp_server_port, ui_port;
	uint32_t latency_time, discover_time;
	float_t ui_refresh_time;
	po::options_description desc("Allowed options");
	desc.add_options()
						(",u", po::value<uint16_t>(&udp_server_port)->default_value(3382), "UDP server port")
						(",U", po::value<uint16_t>(&ui_port)->default_value(3637), "UI port")
						(",t", po::value<uint32_t>(&latency_time)->default_value(1), "time between measuring latency")
						(",T", po::value<uint32_t>(&discover_time)->default_value(10), "time between discovering available hosts")
						(",v", po::value<float_t>(&ui_refresh_time)->default_value(1.0), "UI refresh time")
						(",s", "SSH broadcast")
						;

	po::variables_map vm;

	try
	{
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
	}
	catch(po::error& e)
	{
		cerr<<"ERROR: "<<e.what()<<endl;
		cerr<<desc<<endl;
		return 0;
	}
	try
	{
		unsigned int nthreads = boost::thread::hardware_concurrency();
		boost::asio::io_service udp_serv_service;
		boost::asio::io_service dns_serv_service;
		boost::asio::io_service delay_service;

		ServerUDP serverUDP(udp_serv_service, udp_server_port);
		ServerDNS serverDNS(dns_serv_service, discover_time, vm.count("-s"));
		boost::thread serverDNSThread(boost::bind(&boost::asio::io_service::run, &dns_serv_service));
		boost::thread serverUDPThread(boost::bind(&ServerUDP::run_server, &serverUDP));
		TelnetServer server(delay_service, ui_port, ui_refresh_time);
		DelayChecker delcheck(delay_service, latency_time);

		boost::asio::deadline_timer timer_(delay_service);
		DataContainer::dc()->set_timer(&timer_);

		boost::thread delayThread(boost::bind(&boost::asio::io_service::run, &delay_service));
		std::vector<boost::thread*> threads;
		for (size_t i = 0; i < nthreads-3; i++) {
			boost::thread *t = new boost::thread(boost::bind(&boost::asio::io_service::run, &delay_service));
			threads.push_back(t);
		}

		std::cerr<<"WORKING\n";
		serverUDPThread.join();
		serverDNSThread.join();
		delayThread.join();
		for (size_t i = 0; i < threads.size(); i++) {
			threads[i]->join();
			delete threads[i];
		}
	}
	catch (std::exception& e)
	{
		cerr<<e.what()<<endl;
	}
	return 0;
}
