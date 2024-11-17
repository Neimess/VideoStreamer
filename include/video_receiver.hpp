#ifndef VIDEO_RECEIVER_HPP
#define VIDEO_RECEIVER_HPP

#include <opencv2/opencv.hpp>
#include <boost/asio.hpp>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <memory>

using boost::asio::ip::udp;

class VideoReceiver {
public:
    VideoReceiver(unsigned short port,
                  unsigned short targetFPS = 30,
                  unsigned short videoWidth = 1280,
                  unsigned short videoHeight = 720);
    void start();

private:
    void receiveFrames();
    void displayFrames(int videoWidth, int videoHeight, int targetFPS);

    unsigned short port_;
    unsigned short targetFPS_;
    unsigned short videoWidth_;
    unsigned short videoHeight_;
    boost::asio::io_context ioContext_;
    udp::socket socket_;

    static std::queue<cv::Mat> frameQueue;
    static std::mutex queueMutex;
    static std::condition_variable frameCondVar;
    static std::atomic<bool> stopDisplay;

    static constexpr size_t maxQueueSize = 100;
};

#endif // VIDEO_RECEIVER_HPP
