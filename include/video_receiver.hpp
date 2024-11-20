#ifndef VIDEO_RECEIVER_HPP
#define VIDEO_RECEIVER_HPP

#include "udp_receiver.hpp"
#include "tcp_receiver.hpp"
#include <opencv2/opencv.hpp>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <nlohmann/json.hpp>
#include "logger.hpp"
enum class ProtocolType { UDP, TCP };

class VideoReceiver {
public:
    

    VideoReceiver(ProtocolType protocol, unsigned short port,
                  unsigned short targetFPS = 30,
                  unsigned short videoWidth = 1280,
                  unsigned short videoHeight = 720);

    void start();

private:
    void receiveFrames();
    void displayFrames(int videoWidth, int videoHeight, int targetFPS);

    ProtocolType protocol_;
    unsigned short targetFPS_;
    unsigned short videoWidth_;
    unsigned short videoHeight_;

    std::unique_ptr<Receiver> receiver_;
    std::queue<cv::Mat> frameQueue;
    std::mutex queueMutex;
    std::condition_variable frameCondVar;
    std::atomic<bool> stopDisplay;

    static constexpr size_t maxQueueSize = 100;
};

#endif // VIDEO_RECEIVER_HPP
