#include "video_sendler.hpp"
#include "logger.hpp"

VideoSendler::VideoSendler(const std::string& ip, unsigned short port, const std::string& videoSRC, VideoSourceType sourceType, unsigned short targetFPS)
    : ip_(ip), port_(port), videoSRC_(videoSRC), sourceType_(sourceType), stopFlag_(false), socket_(ioContext_, udp::endpoint(udp::v4(), 0)) {
    Logger::getInstance().log("VideoSendler initialized with file source: " + videoSRC_);
}

VideoSendler::VideoSendler(const std::string& ip, unsigned short port, unsigned short cameraIndex, VideoSourceType sourceType, unsigned short targetFPS)
    : ip_(ip), port_(port), cameraIndex_(cameraIndex), sourceType_(sourceType), stopFlag_(false), socket_(ioContext_, udp::endpoint(udp::v4(), 0)) {
    Logger::getInstance().log("VideoSendler initialized with camera source, index: " + std::to_string(cameraIndex_));
}

void VideoSendler::start() {
    Logger::getInstance().log("Starting VideoSendler.");

    std::thread captureThread(&VideoSendler::captureVideo, this);
    std::thread ioThread([this]() { ioContext_.run(); });

    captureThread.join();
    ioThread.join();
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

        sendFrame();
    }

    cap.release();
    Logger::getInstance().log("Video source released.");
}

void VideoSendler::sendFrame() {
    udp::endpoint serverEndpoint(boost::asio::ip::address::from_string(ip_), port_);

    std::lock_guard<std::mutex> lock(frameMutex_);
    if (sharedFrame_.empty()) return;

    cv::imencode(".jpg", sharedFrame_, buffer_);
    uint32_t bufferSize = static_cast<uint32_t>(buffer_.size());


    auto bufferSizePtr = std::make_shared<uint32_t>(bufferSize);
    socket_.async_send_to(
        boost::asio::buffer(bufferSizePtr.get(), sizeof(uint32_t)), serverEndpoint,
        [this, bufferSizePtr, serverEndpoint](const boost::system::error_code& error, std::size_t /*bytes_sent*/) {
            if (!error) {
                auto bufferPtr = std::make_shared<std::vector<uchar>>(buffer_);
                socket_.async_send_to(
                    boost::asio::buffer(*bufferPtr), serverEndpoint,
                    [bufferPtr](const boost::system::error_code& error, std::size_t bytes_sent) {
                        if (error) {
                            Logger::getInstance().log("Error sending frame data: " + std::string(error.message()));
                        } else {
                            Logger::getInstance().log("Frame sent successfully. Size: " + std::to_string(bytes_sent));
                        }
                    });
            } else {
                Logger::getInstance().log("Error sending buffer size: " + std::string(error.message()));
            }
        });
}
