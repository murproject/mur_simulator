cmake_minimum_required(VERSION 3.14)
project(simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

set(OpenCV_DIR "D:/projects/opencv_430/build3")
set(ZeroMQ_DIR "D:/projects/zeromq-4.3.4/build")
set(GSTREAMER_DIR "D:/gstreamer/1.0/msvc_x86_64/bin")
set(URHO3D_HOME "D:/projects/Urho3D-1.7.1/Urho3D-1.7.1/build2")

set(ZMQ_INCLUDE_DIR "D:/projects/zeromq-4.3.4/include")
set(ZMQ_LIBRARY "D:/projects/zeromq-4.3.4/build/lib/Release/libzmq-v142-mt-4_3_4.lib")
set(ZEROMQ "D:/projects/zeromq-4.3.4/build")
include_directories("D:/projects/zeromq-4.3.4/include")

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

include(UrhoCommon)
include(windeployqt)

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
        ${ZMQ_LIBRARIES}
        )

file(GLOB_RECURSE PROJECT_RESOURCES
    ${PROJECT_SOURCE_DIR}/resources/*
    )

#install(FILES ${PROJECT_RESOURCES} DESTINATION share/mur-simulator)

set(OpenCV_DIR_DLL "${OpenCV_DIR}/bin/$<CONFIGURATION>")

add_custom_command(TARGET simulator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OpenCV_DIR_DLL}/opencv_highgui430.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OpenCV_DIR_DLL}/opencv_imgproc430.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OpenCV_DIR_DLL}/opencv_core430.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OpenCV_DIR_DLL}/opencv_imgcodecs430.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OpenCV_DIR_DLL}/opencv_videoio430.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OpenCV_DIR_DLL}/zlib1.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_DIR}/intl-8.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_DIR}/ffi-7.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_DIR}/z-1.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_DIR}/orc-0.4-0.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_DIR}/gstapp-1.0-0.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_DIR}/gstreamer-1.0-0.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_DIR}/gstriff-1.0-0.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_DIR}/gstpbutils-1.0-0.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_DIR}/gstbase-1.0-0.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_DIR}/gsttag-1.0-0.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_DIR}/gstaudio-1.0-0.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_DIR}/gstvideo-1.0-0.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_DIR}/glib-2.0-0.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_DIR}/gobject-2.0-0.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSTREAMER_DIR}/gmodule-2.0-0.dll" $<TARGET_FILE_DIR:simulator>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ZEROMQ}/bin/Release/libzmq-v142-mt-4_3_4.dll" $<TARGET_FILE_DIR:simulator>
    )

windeployqt(simulator)
