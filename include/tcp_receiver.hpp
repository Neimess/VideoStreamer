#ifndef TCP_RECEIVER_HPP
#define TCP_RECEIVER_HPP

#include "receiver.hpp"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class TCPReceiver : public Receiver {
public:
    explicit TCPReceiver(unsigned short port);
    void start() override;
    std::vector<unsigned char> receive() override;

private:
    boost::asio::io_context ioContext_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    bool isConnected_;
};

#endif // TCP_RECEIVER_HPP
