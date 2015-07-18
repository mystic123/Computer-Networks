/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#include "utils.h"
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

uint64_t GetTimeStamp()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*MICRO_MULT+tv.tv_usec;
}

std::string encodeString(const std::string& str)
{
	std::string cpy(" "+str);
	size_t first, second;
	first = str.find(".");
	cpy[0] = (char)first;
	second = str.find(".", first+1);
	while (second < str.size()) {
		cpy[first+1] = (char)(second-first-1);
		first = second;
		second = str.find(".", first+1);
	}
	cpy[first+1] = str.size() - first- 1;
	return cpy;
}

std::string decodeString(const char *str, size_t& end)
{
	std::string addr = "";
	size_t i = 0;
	while (str[i] != 0) {
		size_t l = 0;
		l += (unsigned char)str[i];
		if (l < 64) {
			std::string tmp(str+i+1, l);
			addr += tmp+".";
			i += l+1;
			l = 0;
		} else {
			break;
		}
	}
	end = i;
	return addr;
}
