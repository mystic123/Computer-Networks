/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#ifndef DNS_RESPONSE_H
#define DNS_RESPONSE_H

#include <istream>
#include <ostream>
#include <string>
#include "utils.h"

class dns_response
{
public:
	enum { A = 1, PTR = 12, CNAME = 5 };

	dns_response(const std::string& str, const std::string& rdata, uint16_t type = 1, uint16_t rclass = 1, uint32_t ttl = 120):
		name_(str), rdata_(rdata), type_(type), rclass_(rclass), ttl_(ttl)
	{
	}

	dns_response()
	{
	}

	std::string name() const { return name_; }
	std::string rdata() const { return rdata_; }
	uint16_t type() const { return type_; }
	uint16_t rclass() const { return rclass_; }
	uint32_t ttl() const { return ttl_; }

	void setName(const std::string& str) { name_ = str; }
	void setRdata(const std::string& str) { rdata_ = str; }
	void setType(uint16_t type) { type_ = type; }
	void setRclass(uint16_t rclass) { rclass_ = rclass; }
	void setTtl(uint32_t ttl) { ttl_ = ttl; }

	static dns_response fromString(const char* str)
	{
		size_t i;
		//std::cerr<<"tu1\n";
		std::string name = decodeString(str,i);
		i++;
		uint16_t type = 0;
		uint16_t rclass = 0;
		uint32_t ttl = 0;
		//uint16_t rdlength = 0;
		type = (str[i] << 8) + str[i+1];
		i+=2;
		rclass = (str[i] << 8) + str[i+1];
		i+=2;
		ttl = (str[i] << 24) + (str[i+1] << 16) + (str[i+2] << 8) + str[i+3];
		i+=4;
		//rdlength = (str[i] << 8) + str[i+1];
		i+=2;
		dns_response r;
		r.setName(name);
		r.setType(type);
		r.setRclass(rclass);
		r.setTtl(ttl);
		//std::cerr<<"tu3\n";
		std::string rdata;
		if (type == PTR) {
			rdata = decodeString(str+i,i);
		}
		else {
			rdata = decodeString(str+i-1,i);
		}
		r.setRdata(rdata);
		return r;
	}

	void print()
	{
		std::cerr<<"name: "<<name_<<"\ntype: "<<type_<<"\nrclass: "<<rclass_<<"\nttl: "<<ttl_<<"\nrdata: "<<rdata_<<std::endl;
	}

	friend std::ostream& operator<<(std::ostream& os, const dns_response& response)
	{
		std::string name_copy = encodeString(response.name_);
		os.write(name_copy.c_str(),name_copy.size());

		char a,b,c,d;
		a = (char)(response.type_ >> 8);
		b = (char)(response.type_ & 0x00FF);
		os.write(reinterpret_cast<const char*>(&a), 1);
		os.write(reinterpret_cast<const char*>(&b), 1);
		a = (char)(response.rclass_ >> 8);
		b = (char)(response.rclass_ & 0x00FF);
		os.write(reinterpret_cast<const char*>(&a), 1);
		os.write(reinterpret_cast<const char*>(&b), 1);
		a = (char)(response.ttl_ >> 24);
		b = (char)(response.ttl_ >> 16);
		c = (char)(response.ttl_ >> 8);
		d = (char)(response.ttl_ & 0x000000FF);
		os.write(reinterpret_cast<const char*>(&a), 1);
		os.write(reinterpret_cast<const char*>(&b), 1);
		os.write(reinterpret_cast<const char*>(&c), 1);
		os.write(reinterpret_cast<const char*>(&d), 1);
		std::string s = encodeString(response.rdata_).c_str();
		if (response.type_ == PTR) {
			a = s.size() >> 8;
			b = s.size() & 0x00FF;
			os.write(&a, 1);
			os.write(&b, 1);
		}
		else {
			a = 0;
			os.write(&a, 1);
		}
		os.write(s.c_str(), s.size());
		if (response.type_ == PTR) {
			char x = 0;
			os.write(&x, 1);
		}
		return os;
	}

private:
	std::string name_;
	std::string rdata_;
	uint16_t type_ = 0;
	uint16_t rclass_ = 1;
	uint32_t ttl_ = 120;
};

#endif // DNS_RESPONSE_H
