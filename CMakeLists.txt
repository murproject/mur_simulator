cmake_minimum_required(VERSION 3.14)
project(simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS
        Core
        Widgets
        Gui
        Xml
        Test
        Concurrent
        OpenGL
        )

find_package(OpenCV REQUIRED)
find_package(ZeroMQ REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

include(UrhoCommon)

file(GLOB_RECURSE SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/sources/*.h
        ${PROJECT_SOURCE_DIR}/sources/*.cpp
        )

set(TARGET_NAME simulator)

setup_main_executable()

target_link_libraries(simulator
        ${OpenCV_LIBS}
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        Qt5::Xml
        libzmq
        opengl32
        )

