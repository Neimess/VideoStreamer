#include "tcp_receiver.hpp"
#include "logger.hpp"

TCPReceiver::TCPReceiver(unsigned short port)
    : acceptor_(ioContext_, tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      socket_(ioContext_),
      isConnected_(false) {
    Logger::getInstance().log("TCPReceiver initialized on port " + std::to_string(port));
}


void TCPReceiver::start() {
    Logger::getInstance().log("TCPReceiver started.");

    try {
        Logger::getInstance().log("Waiting for incoming connection...");
        acceptor_.accept(socket_);
        isConnected_ = true;
        Logger::getInstance().log("TCP connection accepted.");
    } catch (const std::exception& e) {
        Logger::getInstance().log(std::string("TCPReceiver start error: ") + e.what());
    }
}

std::vector<unsigned char> TCPReceiver::receive() {
    if (!isConnected_) {
        Logger::getInstance().log("TCPReceiver is not connected. Cannot receive data.");
        return {};
    }

    std::vector<unsigned char> buffer(65536);
    try {
        size_t length = socket_.read_some(boost::asio::buffer(buffer));
        buffer.resize(length);

        auto remoteEndpoint = socket_.remote_endpoint();
        Logger::getInstance().logDetailed(
            "Received TCP data",
            remoteEndpoint.address().to_string(),
            remoteEndpoint.port(),
            length
        );
    } catch (const std::exception& e) {
        Logger::getInstance().log("TCP receive error: " + std::string(e.what()));
    }
    return buffer;
}