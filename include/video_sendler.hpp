#ifndef VIDEO_SENDLER_HPP
#define VIDEO_SENDLER_HPP

#include <opencv2/opencv.hpp>
#include <boost/asio.hpp>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>

using boost::asio::ip::tcp;

enum class VideoSourceType {
    VIDEO_FILE,
    CAMERA
};

class VideoSendler {
public:
    VideoSendler(const std::string& ip, unsigned short port, const std::string& videoSRC, VideoSourceType sourceType, unsigned short targetFPS = 30);
    VideoSendler(const std::string& ip, unsigned short port, unsigned short cameraIndex, VideoSourceType sourceType, unsigned short targetFPS = 30);
    void start();

private:
    void captureVideo();
    void sendVideo();
    bool connectToServer();

    std::string ip_;
    unsigned short port_;
    std::string videoSRC_;
    unsigned short cameraIndex_;
    VideoSourceType sourceType_;
    std::atomic<bool> stopFlag_;
    boost::asio::io_context ioContext_;
    tcp::socket socket_;
    std::mutex frameMutex_;
    cv::Mat sharedFrame_;
    unsigned short targetFPS_;
};

#endif // VIDEO_SENDLER_HPP
