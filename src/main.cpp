#include <iostream>
#include "video_receiver.hpp"
#include "video_sendler.hpp"
#include "logger.hpp"

#include <boost/asio.hpp>
#include <string>
#include <iostream>

std::string getLocalIPAddress() {
    try {
        boost::asio::io_context ioContext;
        boost::asio::ip::udp::resolver resolver(ioContext);
        boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), "8.8.8.8", "80");
        boost::asio::ip::udp::resolver::iterator endpoints = resolver.resolve(query);
        boost::asio::ip::udp::endpoint endpoint = *endpoints;
        boost::asio::ip::udp::socket socket(ioContext);
        socket.connect(endpoint);

        std::string ipAddress = socket.local_endpoint().address().to_string();
        socket.close();


        return ipAddress;
    } catch (const std::exception& e) {
        std::cerr << "Error getting local IP address: " << e.what() << std::endl;
        return "Unknown";
    }
}


void printUsage() {
    std::cout << "Help:" << std::endl;
    std::cout << "  Receiver: ./VideoStreamer --server <PORT>" << std::endl;
    std::cout << "  Sendler with video file: ./VideoStreamer --client <IP> <PORT> <VIDEO_FILE>" << std::endl;
    std::cout << "  Sendler with camera: ./VideoStreamer --client <IP> <PORT> <CAMERA_INDEX>" << std::endl;
}

int main(int argc, char* argv[]) {


    if (argc == 2 && std::string(argv[1]) == "--help") {
        printUsage();
        return 0;
    }

    if (argc < 3) {
        Logger::getInstance().log("Invalid arguments.");
        printUsage();
        return -1;
    }

    std::string mode;
    std::string ip;
    int port = 0;
    std::string videoSource;
    int cameraIndex = -1;
    bool isServer = false;
    bool isClient = false;
    bool useCamera = false;


    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--server") {
            isServer = true;
        } else if (arg == "--client") {
            isClient = true;
        } else if (arg == "--ip" && i + 1 < argc) {
            ip = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--file" && i + 1 < argc) {
            videoSource = argv[++i];
        } else if (arg == "--camera" && i + 1 < argc) {
            cameraIndex = std::stoi(argv[++i]);
            useCamera = true;
        } else {
            Logger::getInstance().log("Unknown argument: " + arg);
            printUsage();
            return -1;
        }
    }

    if (isServer) {
        if (port == 0) {
            Logger::getInstance().log("Port is required for server mode.");
            printUsage();
            return -1;
        }
        std::string localIP = getLocalIPAddress();
        Logger::getInstance().log("Starting server (receiver) on port: " + std::to_string(port));
        Logger::getInstance().log("Local IPv4 address: " + localIP);
        VideoReceiver receiver(port);
        receiver.start();
    }
    else if (isClient) {
        if (ip.empty() || port == 0) {
            Logger::getInstance().log("IP and port are required for client mode.");
            printUsage();
            return -1;
        }

        if (useCamera) {
            if (cameraIndex < 0) {
                Logger::getInstance().log("Camera index is required for camera mode.");
                printUsage();
                return -1;
            }
            Logger::getInstance().log("Starting client (sendler) with camera source, index: " + std::to_string(cameraIndex));
            VideoSendler sendler(ip, port, cameraIndex, VideoSourceType::CAMERA);
            sendler.start();
        } else if (!videoSource.empty()) {
            Logger::getInstance().log("Starting client (sendler) with video file source: " + videoSource);
            VideoSendler sendler(ip, port, videoSource, VideoSourceType::VIDEO_FILE);
            sendler.start();
        } else {
            Logger::getInstance().log("Video file or camera index is required.");
            printUsage();
            return -1;
        }
    } else {
        Logger::getInstance().log("Invalid mode. Use --server or --client.");
        printUsage();
        return -1;
    }

    Logger::getInstance().log("Application finished.");
    return 0;
}

