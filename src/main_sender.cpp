#include <iostream>
#include <yaml-cpp/yaml.h>
#include "video_sender.hpp"

int main() {
    try {
        // Путь к конфигурационному файлу
        std::string config_path(std::string(CONFIG_DIR) + "/videoConfigure.yaml");
        YAML::Node config = YAML::LoadFile(config_path);

        // Чтение параметров из конфигурации
        unsigned short port = config["port"].as<unsigned short>();
        std::string ip_address = config["ip_address"].as<std::string>();
        std::string protocolType = config["protocolType"].as<std::string>();
        unsigned short cameraIndex = config["videoSource"].as<unsigned short>();

        // Определение протокола
        ProtocolType protocol = (protocolType == "udp") ? ProtocolType::UDP : ProtocolType::TCP;

        // Создание и запуск VideoSender
        VideoSender sender(ip_address, port, cameraIndex, protocol);
        sender.start();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
