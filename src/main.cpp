#include <iostream>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

using boost::asio::ip::udp;

int main() {
    try {
        // Настройки сокета
        boost::asio::io_context io_context;
        udp::socket socket(io_context, udp::endpoint(udp::v4(), 12345)); // Привязываем к порту
        std::vector<uchar> buffer(65507); // Максимальный размер пакета для UDP

        while (true) {
            udp::endpoint sender_endpoint;
            boost::system::error_code error;

            // Получение данных
            size_t length = socket.receive_from(boost::asio::buffer(buffer), sender_endpoint, 0, error);

            if (error && error != boost::asio::error::message_size) {
                throw boost::system::system_error(error);
            }

            // Восстановление изображения из полученных данных
            std::vector<uchar> data(buffer.begin(), buffer.begin() + length);
            cv::Mat frame = cv::imdecode(data, cv::IMREAD_COLOR);

            if (frame.empty()) {
                std::cerr << "Ошибка: Невозможно декодировать изображение!" << std::endl;
                continue;
            }

            // Отображение изображения
            cv::imshow("Received Frame", frame);
            if (cv::waitKey(1) == 27) break; // Нажмите ESC для выхода
        }

        cv::destroyAllWindows();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }

    return 0;
}
