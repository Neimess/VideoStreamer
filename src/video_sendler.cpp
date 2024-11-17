#include "video_sendler.hpp"
#include "logger.hpp"

VideoSendler::VideoSendler(const std::string& ip, unsigned short port, const std::string& videoSRC, VideoSourceType sourceType, unsigned short targetFPS)
    : ip_(ip), port_(port), videoSRC_(videoSRC), sourceType_(sourceType), stopFlag_(false), socket_(ioContext_) {
    Logger::getInstance().log("VideoSendler initialized with file source: " + videoSRC_);
}

VideoSendler::VideoSendler(const std::string& ip, unsigned short port, unsigned short cameraIndex, VideoSourceType sourceType, unsigned short targetFPS)
    : ip_(ip), port_(port), cameraIndex_(cameraIndex), sourceType_(sourceType), stopFlag_(false), socket_(ioContext_) {
    Logger::getInstance().log("VideoSendler initialized with camera source, index: " + std::to_string(cameraIndex_));
}

void VideoSendler::start() {
    Logger::getInstance().log("Starting VideoSendler.");

    std::thread captureThread(&VideoSendler::captureVideo, this);
    std::thread sendThread(&VideoSendler::sendVideo, this);

    captureThread.join();
    sendThread.join();
}

void VideoSendler::captureVideo() {
    cv::VideoCapture cap;
    if (sourceType_ == VideoSourceType::VIDEO_FILE) {
        cap.open(videoSRC_);
    } else if (sourceType_ == VideoSourceType::CAMERA) {
        cap.open(cameraIndex_);
    }

    if (!cap.isOpened()) {
        Logger::getInstance().log("Failed to open video source.");
        stopFlag_ = true;
        return;
    }

    Logger::getInstance().log("Video source opened.");
    int delay = 1000 / targetFPS_;

    while (!stopFlag_) {
        cv::Mat frame;
        if (!cap.read(frame)) {
            Logger::getInstance().log("Failed to read frame. Stopping.");
            stopFlag_ = true;
            break;
        }

        {
            std::lock_guard<std::mutex> lock(frameMutex_);
            sharedFrame_ = frame.clone();
        }

        Logger::getInstance().log("Captured frame.");
        cv::imshow("Sending Video", sharedFrame_);
        if (cv::waitKey(delay) == 27) {
            Logger::getInstance().log("ESC key pressed. Stopping video capture.");
            stopFlag_ = true;
            break;
        }
    }

    cap.release();
    Logger::getInstance().log("Video source released.");
}

void VideoSendler::sendVideo() {
    if (!connectToServer()) {
        stopFlag_ = true;
        return;
    }

    std::vector<uchar> buffer;
    while (!stopFlag_) {
        cv::Mat frame;
        {
            std::lock_guard<std::mutex> lock(frameMutex_);
            if (sharedFrame_.empty()) continue;
            frame = sharedFrame_.clone();
        }

        cv::imencode(".jpg", frame, buffer);
        uint32_t bufferSize = static_cast<uint32_t>(buffer.size());

        try {
            boost::asio::write(socket_, boost::asio::buffer(&bufferSize, sizeof(bufferSize)));
            boost::asio::write(socket_, boost::asio::buffer(buffer));

            auto now = std::chrono::system_clock::now();
            std::time_t timeNow = std::chrono::system_clock::to_time_t(now);
            Logger::getInstance().log("Frame sent at: " + std::string(std::ctime(&timeNow)) + " Size: " + std::to_string(bufferSize));

            char ack;
            boost::asio::read(socket_, boost::asio::buffer(&ack, sizeof(ack)));
            if (ack != 'T') {
                Logger::getInstance().log("Acknowledgment failed. Stopping.");
                stopFlag_ = true;
            }
        } catch (const std::exception& e) {
            Logger::getInstance().log("Data transmission error: " + std::string(e.what()));
            stopFlag_ = true;
            break;
        }
    }

    socket_.close();
    Logger::getInstance().log("Connection closed.");
}

bool VideoSendler::connectToServer() {
    try {
        tcp::endpoint serverEndpoint(boost::asio::ip::address::from_string(ip_), port_);
        socket_.connect(serverEndpoint);
        Logger::getInstance().log("Connected to server: " + ip_ + ":" + std::to_string(port_));
        return true;
    } catch (const std::exception& e) {
        Logger::getInstance().log("Connection error: " + std::string(e.what()));
        return false;
    }
}
