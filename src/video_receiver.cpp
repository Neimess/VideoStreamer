#include "video_receiver.hpp"
#include "logger.hpp"
#include "metadata.hpp"

VideoReceiver::VideoReceiver(ProtocolType protocol, unsigned short port,
                             unsigned short targetFPS,
                             unsigned short videoWidth,
                             unsigned short videoHeight)
    : protocol_(protocol),
      targetFPS_(targetFPS),
      videoWidth_(videoWidth),
      videoHeight_(videoHeight) {
    if (protocol_ == ProtocolType::UDP) {
        receiver_ = std::make_unique<UDPReceiver>(port);
        Logger::getInstance().log("VideoReceiver initialized with UDP protocol on port " + std::to_string(port));
    } else if (protocol_ == ProtocolType::TCP) {
        receiver_ = std::make_unique<TCPReceiver>(port);
        Logger::getInstance().log("VideoReceiver initialized with TCP protocol on port " + std::to_string(port));
    }
}

void VideoReceiver::start() {
    Logger::getInstance().log("VideoReceiver started.");
    stopDisplay = false;

    if (protocol_ == ProtocolType::TCP) {
        Logger::getInstance().log("Waiting for TCP connection...");
        auto* tcpReceiver = dynamic_cast<TCPReceiver*>(receiver_.get());
        if (tcpReceiver) {
            tcpReceiver->start(); // Прямой вызов ожидания подключения
            Logger::getInstance().log("TCP connection established.");
        } else {
            Logger::getInstance().log("Error: Receiver is not a TCPReceiver.");
            return;
        }
    }

    Logger::getInstance().log("Starting threads.");
    std::thread receiveThread(&VideoReceiver::receiveFrames, this);
    std::thread displayThread(&VideoReceiver::displayFrames, this, videoWidth_, videoHeight_, targetFPS_);

    receiveThread.join();
    displayThread.join();
}


void VideoReceiver::receiveFrames() {
    while (!stopDisplay) {
        std::vector<unsigned char> data = receiver_->receive();

        if (!data.empty()) {
            auto separatorPos = std::find(data.begin(), data.end(), 0);
            if (separatorPos == data.end()) {
                Logger::getInstance().log("Error: No metadata separator found!");
                continue;
            }

            std::vector<unsigned char> metadataBytes(data.begin(), separatorPos);
            MetaData metaData;
            metaData.parse(metadataBytes);
            if (!metaData.isValid()) {
                continue;
            }

            std::vector<unsigned char> frameData(separatorPos + 1, data.end());
            cv::Mat frame = cv::imdecode(frameData, cv::IMREAD_COLOR);
            if (!frame.empty()) {
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    if (frameQueue.size() < maxQueueSize) {
                        frameQueue.push(frame);
                    } else {
                        Logger::getInstance().log("Frame queue is full. Dropping frame.");
                    }
                }
                frameCondVar.notify_one();
            }
        }
    }
}

void VideoReceiver::displayFrames(int videoWidth, int videoHeight, int targetFPS) {
    cv::namedWindow("VideoReceiver", cv::WINDOW_NORMAL);
    cv::resizeWindow("VideoReceiver", videoWidth, videoHeight);

    while (!stopDisplay) {
        std::unique_lock<std::mutex> lock(queueMutex);
        frameCondVar.wait(lock, [this]() {
            return !frameQueue.empty() || stopDisplay;
        });

        if (stopDisplay) break;

        if (!frameQueue.empty()) {
            cv::Mat frame = frameQueue.front();
            frameQueue.pop();
            lock.unlock();

            cv::imshow("VideoReceiver", frame);

            if (cv::waitKey(1) == 27) {
                Logger::getInstance().log("Stop signal received.");
                stopDisplay = true;
            }
        }
    }
    cv::destroyWindow("VideoReceiver");
}
