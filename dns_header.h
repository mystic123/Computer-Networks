/*
 *	Paweł Kapica, 334579
 *	Sieci Komputerowe
 *	Duże zadanie zaliczeniowe
 *
 */
#ifndef DNS_HEADER_H
#define DNS_HEADER_H

#include <istream>
#include <ostream>
//
// Format:
//                                 1  1  1  1  1  1
//   0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
//  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//  |                      ID                       |
//  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//  |QR|   OpCode  |AA|TC|RD|RA| Z|AD|CD|   RCODE   |
//  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//  |                QDCOUNT/ZOCOUNT                |
//  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//  |                ANCOUNT/PRCOUNT                |
//  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//  |                NSCOUNT/UPCOUNT                |
//  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//  |                    ARCOUNT                    |
//  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

class dns_header
{
public:
	static const size_t LENGTH = 12;
	enum opcode {
		query = 0, iquery = 1, status = 2
	};

	enum type {
		query_type = 0, response = 1
	};

	enum rcode {
		noerror = 0, formaterror = 1, servererror = 2, nameerror = 3, notimplemented = 4, rejected = 5
	};

	dns_header()
	{
		std::fill(t, t + sizeof(t), 0);
	}

	uint16_t id() const { return (t[0] << 8) + t[1]; }
	char qr() const { return (t[2] >> 7); }
	char opcode() const { return (t[2] >> 6); }
	char aa() const { return (t[2] >> 2); }
	char tc() const { return (t[2] >> 1); }
	char rd() const { return t[2]; }
	char ra() const { return (t[3] >> 7); }
	char z() const { return (t[3] >> 6); }
	char rcode() const { return t[3]; }
	uint16_t qdcount() const { return (t[4] << 8) + t[5]; }
	uint16_t ancount() const { return (t[6] << 8) + t[7]; }
	uint16_t nscount() const { return (t[8] << 8) + t[9]; }
	uint16_t arcount() const { return (t[10] << 8) + t[11]; }

	void setId(uint16_t x) { t[0] = (char)((x & 0xFF00) >> 8);
								t[1] = (char)(x & 0x00FF); }
	void setQr(char x) { t[2] |= (x << 7); }
	void setOpcode(char x) { t[2] |= (x << 6); }
	void setAa(char x) { t[2] |= (x << 2); }
	void setTc(char x) { t[2] |= (x << 1); }
	void setRd(char x) { t[2] |= x; }
	void setRa(char x) { t[3] |= (x << 6); }
	void setZ(char x) { t[3] |= (x << 5); }
	void setRcode(char x) { t[3] |= x; }
	void setQdcount(uint16_t x) { t[4] = (char)(x & 0xFF00);
									t[5] = (char)(x & 0x00FF);}
	void setAncount(uint16_t x) { t[6] = (char)(x & 0xFF00);
									t[7] = (char)(x & 0x00FF); }
	void setNscount(uint16_t x) { t[8] = (char)(x & 0xFF00);
									t[9] = (char)(x & 0x00FF); }
	void setArcount(uint16_t x) { t[10] = (char)(x & 0xFF00);
									t[11] = (char)(x & 0x00FF); }

	friend std::istream& operator>>(std::istream& is, dns_header& header)
	{
		return is.read(reinterpret_cast<char *>(header.t), LENGTH);
	}

	friend std::ostream& operator<<(std::ostream& os, const dns_header& header)
	{
		return os.write(reinterpret_cast<const char*>(header.t), LENGTH);
	}

	static dns_header fromString(const char* str)
	{
		dns_header hdr;
		for (size_t i = 0; i < LENGTH; i++) {
			hdr.t[i] = str[i];
		}
		return hdr;
	}

	void print()
	{
		std::cerr<<"id: "<<id()<<"\nqr: "<<qr()<<"\nqdcount: "<<qdcount()<<"\n"<<"ancount: "<<ancount()<<"\n"<<"nscount: "<<nscount()<<"\narcount: "<<arcount()<<"\n";
	}

private:
	unsigned char t[dns_header::LENGTH];
};

#endif // DNS_HEADER_H
