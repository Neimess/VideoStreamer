#include "udp_sender.hpp"
#include "logger.hpp"

UDPSender::UDPSender(const std::string& address, unsigned short port)
    : socket_(ioContext_, udp::endpoint(udp::v4(), 0)),
      endpoint_(boost::asio::ip::make_address(address), port) {
    Logger::getInstance().log("UDPSender initialized for " + address + ":" + std::to_string(port));
}

void UDPSender::start() {
    Logger::getInstance().log("UDPSender started.");
}

void UDPSender::send(const std::vector<unsigned char>& data) {
    socket_.send_to(boost::asio::buffer(data), endpoint_);
}
