#include "tcp_sender.hpp"
#include "logger.hpp"

TCPSender::TCPSender(const std::string& address, unsigned short port)
    : socket_(ioContext_),
      endpoint_(boost::asio::ip::make_address(address), port),
      isConnected_(false) {
    Logger::getInstance().log("TCPSender initialized for " + address + ":" + std::to_string(port));
}

TCPSender::~TCPSender() {
    try {
        if (socket_.is_open()) {
            socket_.close();
            Logger::getInstance().log("TCP socket closed.");
        }
    } catch (const std::exception& e) {
        Logger::getInstance().log("Error closing socket: " + std::string(e.what()));
    }
}

void TCPSender::start() {
    try {
        socket_.connect(endpoint_);
        isConnected_=true;
        Logger::getInstance().log("TCP connection established with " +
                                   endpoint_.address().to_string() + ":" +
                                   std::to_string(endpoint_.port()));
    } catch (const boost::system::system_error& e) {
        Logger::getInstance().log("TCP connection error: " + std::string(e.what()));
        throw;
    }
}


void TCPSender::send(const std::vector<unsigned char>& data) {
    try {
        if (isConnected_ && socket_.is_open()) {
            boost::asio::write(socket_, boost::asio::buffer(data));
        }
    } catch (const boost::system::system_error& e) {
        Logger::getInstance().log("Error in TCPSender::send: " + std::string(e.what()));
        isConnected_ = false; // Сбрасываем флаг, если соединение разорвано
    }
}
