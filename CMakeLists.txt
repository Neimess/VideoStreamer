cmake_minimum_required(VERSION 3.27)
project(VideoStreamer)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 20)
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

# Указываем директорию для заголовочных файлов
include_directories(include)

# Устанавливаем директорию для выходных файлов
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(COMMON_FLAGS " -Wpedantic -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion -Wdouble-promotion")

# Флаги компиляции для отладочной и релизной сборки
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 ${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG ${COMMON_FLAGS}")
# Платформозависимая линковка
if (WIN32)
    list(APPEND ADDITIONAL_LIBS ws2_32) # Для Windows добавляем ws2_32 (WinSock)
elseif (UNIX)
    list(APPEND ADDITIONAL_LIBS pthread) # Для Linux добавляем pthread
endif()

# Устанавливаем пути к исходным файлам
set(SOURCES
    src/main.cpp
    src/video_receiver.cpp
    src/video_sendler.cpp
    src/logger.cpp
)

# Создаем исполняемый файл
add_executable(VideoStreamer ${SOURCES})

# Подключаем заголовочные файлы и библиотеки к исполняемым файлам
target_include_directories(VideoStreamer PRIVATE ${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

# Линкуем библиотеки
target_link_libraries(VideoStreamer PRIVATE ${OpenCV_LIBS} Boost::system Boost::thread ${ADDITIONAL_LIBS})

# Платформозависимые настройки для Windows и Linux
if (WIN32)
    message(STATUS "Building for Windows")
    target_compile_definitions(VideoStreamer PRIVATE -D_WIN32_WINNT=0x0601) # Windows 7 и выше
elseif (UNIX)
    message(STATUS "Building for Linux")
    target_compile_options(VideoStreamer PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Для Qt6 автоматическая сборка MOC-файлов
if (Qt6Widgets_FOUND)
    qt_standard_project_setup()
endif()
