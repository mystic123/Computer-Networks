#include "clienticmp.h"
#include "ipv4_header.h"
#include "icmp_header.h"
#include "datacontainer.h"

const std::string ClientICMP::BODY = ClientICMP::initialize_body();

ClientICMP::ClientICMP(boost::asio::io_service& io_service, std::string dest): resolver_(io_service),
		socket_(io_service, boost::asio::ip::icmp::v4()), timer_(io_service), dest_(dest)
{
	boost::asio::ip::icmp::resolver::query query(boost::asio::ip::icmp::v4(), dest, "");
	destination_ = *resolver_.resolve(query);
}

void ClientICMP::measure_latency()
{
	start_send();
	start_receive();
}

void ClientICMP::start_send()
{
	// Create an ICMP header for an echo request.
	icmp_header echo_request;
	echo_request.type(icmp_header::echo_request);
	echo_request.code(0);
	echo_request.identifier(0x13);
	echo_request.sequence_number(0);
	compute_checksum(echo_request, BODY.begin(), BODY.end());

	// Encode the request packet.
	boost::asio::streambuf request_buffer;
	std::ostream os(&request_buffer);
	os << echo_request << BODY;

	// Send the request.
	time_sent_ = GetTimeStamp();
	socket_.async_send_to(request_buffer.data(), destination_,
			bind(&ClientICMP::handle_send, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}

void ClientICMP::handle_send(const boost::system::error_code& error, size_t bytes)
{
	// Wait up to five seconds for a reply.
	timer_.expires_from_now(boost::posix_time::seconds(MAX_DELAY));
	timer_.async_wait(boost::bind(&ClientICMP::handle_timeout, this, boost::asio::placeholders::error));
}

void ClientICMP::handle_timeout(const boost::system::error_code& error)
{
	if (!error) {
		DataContainer::dc()->insertMeasure(boost::asio::ip::address_v4::from_string(dest_), measure_type::ICMP, MAX_DELAY*MICRO_MULT);
		mtx_.lock();
		finished_ = true;
		mtx_.unlock();
	}
}

void ClientICMP::start_receive()
{
	// Discard any data already in the buffer.
	reply_buffer_.consume(reply_buffer_.size());

	// Wait for a reply. We prepare the buffer to receive up to 64KB.
	socket_.async_receive(reply_buffer_.prepare(65536),
			boost::bind(&ClientICMP::handle_receive, this, _2));
}

void ClientICMP::handle_receive(std::size_t length)
{
	// The actual number of bytes received is committed to the buffer so that we
	// can extract it using a std::istream object.
	reply_buffer_.commit(length);

	// Decode the reply packet.
	std::istream is(&reply_buffer_);
	ipv4_header ipv4_hdr;
	icmp_header icmp_hdr;
	is >> ipv4_hdr >> icmp_hdr;

	// We can receive all ICMP packets received by the host, so we need to
	// filter out only the echo replies that match the our identifier and
	// expected sequence number.
	if (is && icmp_hdr.type() == icmp_header::echo_reply && icmp_hdr.identifier() == 0x13) {
		// If this is the first reply, interrupt the five second timeout.
		timer_.cancel();

		// Print out some information about the reply packet.
		DataContainer::dc()->insertMeasure(boost::asio::ip::address_v4::from_string(dest_), measure_type::ICMP, GetTimeStamp() - time_sent_);
		mtx_.lock();
		finished_ = true;
		mtx_.unlock();
	}
}

std::string ClientICMP::initialize_body()
{
	std::string body_;
	body_.resize(4);
	char d = 0;
	//index
	d += 3;
	d = d << 4;
	d += 3;
	body_[0] = d;
	d = 0;
	d += 4;
	d = d << 4;
	d += 5;
	body_[1] = d;
	d = 0;
	d += 7;
	d = d << 4;
	d += 9;
	body_[2] = d;
	//group
	d = 4;
	body_[3] = d;
	return body_;
}

bool ClientICMP::finished() {
	mtx_.lock();
	auto ret = finished_;
	mtx_.unlock();
	return ret;
}
