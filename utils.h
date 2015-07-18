/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <boost/array.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/program_options.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

enum measure_type { UDP = 0, TCP = 1, ICMP = 2 };

const uint64_t MICRO_MULT = 1000000;

const uint16_t MULTICAST_PORT = 5353;

const uint32_t MAX_DELAY = 10;

uint64_t GetTimeStamp();

std::string encodeString(const std::string& str);

std::string decodeString(const char *str, size_t& end);

#endif //UTILS.H
