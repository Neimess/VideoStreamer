cmake_minimum_required(VERSION 3.27)
project(VideoStreamer)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настраиваем пути к OpenCV и Boost
if (WIN32)
    # Пути для Windows
    set(OpenCV_DIR "C:/msys64/mingw64/lib/cmake/opencv4")
    set(Boost_DIR "C:/msys64/mingw64/lib/cmake/Boost-1.86.0")
elseif (UNIX)
    # Пути для Linux (здесь можно указать пути, если они нестандартные)
    # set(OpenCV_DIR "/usr/local/lib/cmake/opencv4") # Пример для Linux
endif()

# Находим пакеты OpenCV, Qt6 и Boost
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs highgui videoio)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Boost REQUIRED COMPONENTS system thread)

# Проверяем, что OpenCV и Boost найдены
if (NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found!")
endif()

if (NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found!")
endif()
# Указываем путь к конфигурационной директории
set(CONFIG_DIR "${CMAKE_SOURCE_DIR}/config")
# Указываем директорию для заголовочных файлов
include_directories(${CMAKE_SOURCE_DIR}/include)

# Устанавливаем директорию для выходных файлов
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(COMMON_FLAGS "-Wall -Wpedantic -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion -Wdouble-promotion")

# Флаги компиляции для отладочной и релизной сборки
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 ${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG ${COMMON_FLAGS}")
# Добавляем директиву препроцессора для пути к конфигурации
add_definitions(-DCONFIG_DIR=\"${CONFIG_DIR}\")
# Платформозависимая линковка
if (WIN32)
    list(APPEND ADDITIONAL_LIBS ws2_32) # Для Windows добавляем ws2_32 (WinSock)
elseif (UNIX)
    list(APPEND ADDITIONAL_LIBS pthread) # Для Linux добавляем pthread
endif()

# Устанавливаем пути к исходным файлам
set(SOURCES_RECEIVER
    src/tcp_receiver.cpp
    src/udp_receiver.cpp
    src/video_receiver.cpp
    src/logger.cpp
    src/metadata.cpp
    src/main_receiver.cpp
)
set(SOURCES_SENDER
    src/tcp_sender.cpp
    src/udp_sender.cpp
    src/video_sender.cpp
    src/logger.cpp
    src/main_sender.cpp
    src/metadata.cpp
    )

    
add_executable(videoReceiver ${SOURCES_RECEIVER})
add_executable(videoSender ${SOURCES_SENDER})

# Подключаем заголовочные файлы и библиотеки к исполняемым файлам
target_include_directories(videoReceiver PRIVATE ${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_include_directories(videoSender PRIVATE ${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
# Линкуем библиотеки
target_link_libraries(videoReceiver PRIVATE ${OpenCV_LIBS} Boost::system Boost::thread ${ADDITIONAL_LIBS} yaml-cpp)
target_link_libraries(videoSender PRIVATE ${OpenCV_LIBS} Boost::system Boost::thread ${ADDITIONAL_LIBS} yaml-cpp)
# Платформозависимые настройки для Windows и Linux
if (WIN32)
    message(STATUS "Building for Windows")
    target_compile_definitions(videoReceiver PRIVATE -D_WIN32_WINNT=0x0601)
    target_compile_definitions(videoSender PRIVATE -D_WIN32_WINNT=0x0601)
elseif (UNIX)
    message(STATUS "Building for Linux")
    target_compile_options(videoReceiver PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(videoSender PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Для Qt6 автоматическая сборка MOC-файлов
if (Qt6Widgets_FOUND)
    qt_standard_project_setup()
endif()
