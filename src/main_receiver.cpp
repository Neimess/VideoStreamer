#include <iostream>
#include <yaml-cpp/yaml.h>
#include "video_receiver.hpp"

int main() {
    std::string config_path = std::string(CONFIG_DIR) + "/videoConfigure.yaml";;
    YAML::Node config = YAML::LoadFile(config_path);

    unsigned short port = config["port"].as<unsigned short>();
    unsigned short targetFPS = config["targetFPS"].as<unsigned short>();
    unsigned short videoWidth = config["videoWidth"].as<unsigned short>();
    unsigned short videoHeight = config["videoHeight"].as<unsigned short>();
    std::string protocolType = config["protocolType"].as<std::string>();
    ProtocolType protocol = (protocolType == "udp") ? ProtocolType::UDP : ProtocolType::TCP;

    VideoReceiver receiver(protocol, port, targetFPS, videoWidth, videoHeight);
    receiver.start();

    return 0;
}
