#ifndef TCP_SENDER_HPP
#define TCP_SENDER_HPP

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include "sender.hpp"

class TCPSender : public Sender{
public:
    TCPSender(const std::string& address, unsigned short port);
    ~TCPSender();

    void start() override;
    void send(const std::vector<unsigned char>& data) override;

private:
    boost::asio::io_context ioContext_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::endpoint endpoint_;
    bool isConnected_;
};


#endif // TCP_SENDER_HPP
