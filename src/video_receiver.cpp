#include "video_receiver.hpp"
#include "logger.hpp"

std::queue<cv::Mat> VideoReceiver::frameQueue;
std::mutex VideoReceiver::queueMutex;
std::condition_variable VideoReceiver::frameCondVar;
std::atomic<bool> VideoReceiver::stopDisplay = false;

VideoReceiver::VideoReceiver(unsigned short port, unsigned short targetFPS, unsigned short videoWidth, unsigned short videoHeight)
    : port_(port), targetFPS_(targetFPS), videoWidth_(videoWidth), videoHeight_(videoHeight),
      socket_(ioContext_, udp::endpoint(udp::v4(), port_)) {}

void VideoReceiver::start() {
    Logger::getInstance().log("Server starts, waiting for senders...");

    std::thread displayThread([this]() {
        displayFrames(videoWidth_, videoHeight_, targetFPS_);
    });
    displayThread.detach();
    receiveFrames();
    Logger::getInstance().log("Server stopped.");
}

void VideoReceiver::receiveFrames() {
    std::vector<uchar> buffer;
    size_t estimatedBufferSize = static_cast<size_t>(videoWidth_) * static_cast<size_t>(videoHeight_) * 3;
    buffer.reserve(estimatedBufferSize);
    Logger::getInstance().log("Estimated buffer size: " + std::to_string(estimatedBufferSize));

    udp::endpoint senderEndpoint;

    while (!stopDisplay) {
        try {
            uint32_t bufferSize;
            size_t bytesReceived = socket_.receive_from(
                boost::asio::buffer(&bufferSize, sizeof(bufferSize)), senderEndpoint);

            if (bytesReceived != sizeof(bufferSize)) {
                Logger::getInstance().log("Failed to receive buffer size. Skipping frame.");
                continue;
            }

            if (bufferSize > estimatedBufferSize) {
                Logger::getInstance().log("Received buffer size is too large. Skipping frame.");
                continue;
            }


            buffer.resize(bufferSize);
            bytesReceived = socket_.receive_from(boost::asio::buffer(buffer), senderEndpoint);

            if (bytesReceived != bufferSize) {
                Logger::getInstance().log("Incomplete frame data received. Skipping frame.");
                continue;
            }

            cv::Mat frame = cv::imdecode(buffer, cv::IMREAD_COLOR);
            if (frame.empty()) {
                Logger::getInstance().log("Failed to decode frame. Skipping.");
                continue;
            }

            {
                std::lock_guard<std::mutex> lock(queueMutex);
                if (frameQueue.size() < maxQueueSize) {
                    frameQueue.push(frame);
                }
            }
            frameCondVar.notify_one();
        } catch (const std::exception &e) {
            Logger::getInstance().log("Error receiving frame: " + std::string(e.what()));
        }
    }
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