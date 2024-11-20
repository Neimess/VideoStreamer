#include "udp_receiver.hpp"
#include "logger.hpp"

UDPReceiver::UDPReceiver(unsigned short port)
    : socket_(ioContext_, udp::endpoint(udp::v4(), port)) {
    Logger::getInstance().log("UDPReceiver initialized on port " + std::to_string(port));
}

void UDPReceiver::start() {
    Logger::getInstance().log("UDPReceiver started.");
    try {
        ioContext_.run();
    } catch (const std::exception& e) {
        Logger::getInstance().log("Error starting UDPReceiver: " + std::string(e.what()));
    }
}

std::vector<unsigned char> UDPReceiver::receive() {
    std::vector<unsigned char> buffer(65536); 
    try {
        size_t length = socket_.receive_from(boost::asio::buffer(buffer), senderEndpoint_);
        buffer.resize(length);
    } catch (const std::exception& e) {
        Logger::getInstance().log("Error receiving UDP data: " + std::string(e.what()));
    }
    return buffer;
}
