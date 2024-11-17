#include "video_receiver.hpp"
#include "logger.hpp"

std::queue<cv::Mat> VideoReceiver::frameQueue;
std::mutex VideoReceiver::queueMutex;
std::condition_variable VideoReceiver::frameCondVar;
std::atomic<bool> VideoReceiver::stopDisplay = false;

VideoReceiver::VideoReceiver(unsigned short port, unsigned short targetFPS, unsigned short videoWidth, unsigned short videoHeight)
    : port_(port), targetFPS_(targetFPS), videoWidth_(videoWidth), videoHeight_(videoHeight),
      acceptor_(ioContext_, tcp::endpoint(tcp::v4(), port_)) {}

void VideoReceiver::start() {
    Logger::getInstance().log("Server starts, waiting for senders...");

    std::thread displayThread([this]() {
        displayFrames(videoWidth_, videoHeight_, targetFPS_);
    });
    displayThread.detach();

    while (!stopDisplay) {
        auto socket = std::make_shared<tcp::socket>(ioContext_);
        acceptor_.accept(*socket);
        std::string clientIp = socket->remote_endpoint().address().to_string();
        int clientPort = socket->remote_endpoint().port();
        Logger::getInstance().log("Sender connected: " + clientIp + ":" + std::to_string(clientPort));

        std::thread clientThread(&VideoReceiver::handleClient, this, socket);
        clientThread.detach();
    }

    Logger::getInstance().log("Server stopped.");
}

void VideoReceiver::handleClient(std::shared_ptr<tcp::socket> socket) {
    std::vector<uchar> buffer;
    size_t estimatedBufferSize = static_cast<size_t>(videoWidth_) * static_cast<size_t>(videoHeight_) * 3; // 3 
    buffer.reserve(estimatedBufferSize);
    Logger::getInstance().log("Estimated buffer size: " + std::to_string(estimatedBufferSize));
    cv::Mat frame;

    try {
        while (!stopDisplay) {
            uint32_t bufferSize;
            boost::asio::read(*socket, boost::asio::buffer(&bufferSize, sizeof(bufferSize)));

            buffer.resize(bufferSize);
            boost::asio::read(*socket, boost::asio::buffer(buffer));

            frame = cv::imdecode(buffer, cv::IMREAD_COLOR);
            if (frame.empty()) {
                Logger::getInstance().log("Failed to decode frame. Skipping.");
                continue;
            }
            boost::asio::write(*socket, boost::asio::buffer("T", 1));

            {
                std::lock_guard<std::mutex> lock(queueMutex);
                if (frameQueue.size() < maxQueueSize) {
                    frameQueue.push(frame);
                }
            }
            frameCondVar.notify_one();
        }
    } catch (const std::exception& e) {
        Logger::getInstance().log("Client handling error: " + std::string(e.what()));
    }

    socket->close();
    Logger::getInstance().log("Client connection closed.");
}

void VideoReceiver::displayFrames(int videoWidth, int videoHeight, int targetFPS) {
    cv::namedWindow("handledCapture", cv::WINDOW_NORMAL);
    cv::resizeWindow("handledCapture", videoWidth, videoHeight);
    int delay = 1000 / targetFPS;

    while (!stopDisplay) {
        cv::Mat frame;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            frameCondVar.wait(lock, [] { return !frameQueue.empty() || stopDisplay; });

            if (stopDisplay) break;

            frame = frameQueue.front();
            frameQueue.pop();
        }

        if (!frame.empty()) {
            cv::imshow("handledCapture", frame);
            if (cv::waitKey(1) == 27) {
                stopDisplay = true;
                frameCondVar.notify_all();
                break;
            }
        }
    }

    cv::destroyAllWindows();
}
