#include "video_sender.hpp"
#include "metadata.hpp"

using json = nlohmann::json;

VideoSender::VideoSender(const std::string& address, unsigned short port,
                         unsigned short cameraIndex, ProtocolType protocol)
    : address_(address), port_(port), cameraIndex_(cameraIndex), protocol_(protocol) {
    if (protocol_ == ProtocolType::TCP) {
        sender_ = std::make_unique<TCPSender>(address, port);
    } else if (protocol_ == ProtocolType::UDP) {
        sender_ = std::make_unique<UDPSender>(address, port);
    }
}



void VideoSender::start() {
    Logger::getInstance().log("Starting VideoSender");

    if (protocol_ == ProtocolType::TCP) {
        Logger::getInstance().log("Establishing TCP connection...");
        auto *tcpSender = dynamic_cast<TCPSender *>(sender_.get());
        if (tcpSender) {
          tcpSender->start();
          Logger::getInstance().log("TCP connection established. Starting threads.");
        } else {
          Logger::getInstance().log("Error: Receiver is not a TCPReceiver");
        }
        
    }

    std::thread captureThread(&VideoSender::captureFrame, this);
    std::thread sendThread(&VideoSender::sendFrame, this);

    captureThread.join();
    sendThread.join();
}


void VideoSender::captureFrame() {
    cv::VideoCapture cap(cameraIndex_); // Открываем камеру

    if (!cap.isOpened()) {
        Logger::getInstance().log("Failed to open camera");
        stopFlag = true;
        return;
    }

    Logger::getInstance().log("Camera opened");

    while (!stopFlag) {
        cv::Mat frame;

        if (!cap.read(frame)) {
            Logger::getInstance().log("Failed to read frame from camera");
            stopFlag = true;
            break;
        }

        auto framePtr = std::make_shared<cv::Mat>(std::move(frame));
        {
            std::lock_guard<std::mutex> lock(frameQueueMutex_);
            if (frameQueue_.size() < maxQueueSize_) {
                frameQueue_.push(framePtr);
                frameCondVar_.notify_one();
            }
        }
        
    }

    cap.release();
}



void VideoSender::sendFrame() {
    while (!stopFlag) {
        std::shared_ptr<cv::Mat> frameToSend;

        {
            std::unique_lock<std::mutex> lock(frameQueueMutex_);
            frameCondVar_.wait(lock, [this]() { return !frameQueue_.empty() || stopFlag; });

            if (stopFlag && frameQueue_.empty()) break;

            frameToSend = frameQueue_.front();
            frameQueue_.pop();
        }

        if (frameToSend) {
            std::vector<unsigned char> encodedBuffer;
            if (!cv::imencode(".jpg", *frameToSend, encodedBuffer, compression_params_)) {
                Logger::getInstance().log("Error: Failed to compress the image!");
                continue;
            }

            MetaData metaData;
            metaData.create(
                nlohmann::json{
                {"frame_size", encodedBuffer.size()},
                {"timestamp", 1234567890}
                });

            if (!metaData.isValid()) {
                Logger::getInstance().log("Error: Failed to create metadata!");
                continue;
            }

            std::string metadataStr = metaData.get().dump();

            std::vector<unsigned char> dataToSend(metadataStr.begin(), metadataStr.end());
            dataToSend.push_back(0);
            dataToSend.insert(dataToSend.end(), encodedBuffer.begin(), encodedBuffer.end());

            // Отправка данных
            sender_->send(dataToSend);
        }
    }
}

void VideoSender::stop() {
    stopFlag = true;
    frameCondVar_.notify_all();
}

