#ifndef UDP_SENDER_HPP
#define UDP_SENDER_HPP

#include "sender.hpp"
#include <boost/asio.hpp>

using boost::asio::ip::udp;

class UDPSender : public Sender {
public:
    UDPSender(const std::string& address, unsigned short port);
    void start() override;
    void send(const std::vector<unsigned char>& data) override;

private:
    boost::asio::io_context ioContext_;
    udp::socket socket_;
    udp::endpoint endpoint_;
};

#endif // UDP_SENDER_HPP
