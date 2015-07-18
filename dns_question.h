/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#ifndef DNS_QUESTION_H
#define DNS_QUESTION_H

#include <istream>
#include <ostream>
#include <string>
#include "utils.h"

class dns_question
{
public:
	enum { A = 1, PTR = 12 };

	dns_question(const std::string& addr, uint16_t type, uint16_t qclass = 1 /* INTERNET */): addr_(addr), type_(type), qclass_(qclass)
	{
	}

	std::string addr() const { return addr_; }
	char type() const { return type_; }
	char qclass() const { return qclass_; }

	void setAddr(std::string addr) { addr_ = addr; }
	void setType(char type) { type_ = type; }
	void setClass(uint16_t qclass) { qclass_ = qclass; }

	void setUnicastResponse(bool v)
	{
		if (v) {
			qclass_ |= 0x8000;
		}
		else {
			uint16_t nval = 0;
			nval = (qclass_ & 0x7FFF);
			qclass_ = nval;
		}
	}

	bool requiresUnicastResponse() { return qclass_ >> 15; }

	static dns_question fromString(const char* str)
	{
		size_t i;
		std::string addr = decodeString(str,i);
		i++;
		uint16_t type;
		uint16_t qclass;
		type = (str[i] << 8) + str[i+1];
		i+=2;
		qclass = (str[i] << 8) + str[i+1];
		dns_question q(addr, type, qclass);
		return q;
	}

	void print()
	{
		std::cerr<<"addr: "<<addr_<<"\ntype: "<<type_<<"\nqclass: "<<qclass_<<std::endl;
	}

	size_t size()
	{
		return encodeString(addr_).size() + 4;
	}

	friend std::ostream& operator<<(std::ostream& os, const dns_question& question)
	{
		std::string addr_copy = encodeString(question.addr_);
		os.write(addr_copy.c_str(),strlen(addr_copy.c_str()));
		
		char a,b;
		a = 0;
		os.write(reinterpret_cast<const char*>(&a), 1);
		a = (char)(question.type_ >> 8);
		b = (char)(question.type_ & 0x00FF);
		os.write(reinterpret_cast<const char*>(&a), 1);
		os.write(reinterpret_cast<const char*>(&b), 1);
		a = (char)(question.qclass_ >> 8);
		b = (char)(question.qclass_ & 0x00FF);
		os.write(reinterpret_cast<const char*>(&a), 1);
		os.write(reinterpret_cast<const char*>(&b), 1);
		
		return os;
	}

private:
	std::string addr_;
	uint16_t type_;
	uint16_t qclass_;
};

#endif // DNS_QUESTION_H
