#ifndef UDP_RECEIVER_HPP
#define UDP_RECEIVER_HPP

#include "receiver.hpp"
#include <boost/asio.hpp>

using boost::asio::ip::udp;

class UDPReceiver : public Receiver {
public:
    UDPReceiver(unsigned short port);
    void start() override;

    std::vector<unsigned char> receive() override;

private:
    boost::asio::io_context ioContext_;
    udp::socket socket_;
    udp::endpoint senderEndpoint_;
};

#endif // UDP_RECEIVER_HPP
