cmake_minimum_required(VERSION 3.14)
project(simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(WEB_SIM "Build for web checker" OFF)

if(WEB_SIM)
    add_compile_definitions(WEB_SIM)
else()
    add_compile_definitions(DESKTOP_SIM)
endif()


if (WIN32)
    include_directories(${ZMQ_INCLUDE_DIR})
endif()

include_directories(${CMAKE_SOURCE_DIR}/resources/libraries/kuba-zip)

find_package(Qt6 REQUIRED COMPONENTS
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

if(WIN32)
    find_package(Urho3D REQUIRED)
endif()

include(UrhoCommon)

file(GLOB_RECURSE SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/sources/*.h
    ${PROJECT_SOURCE_DIR}/sources/*.cpp
)

if (NOT WEB_SIM)
    file(GLOB_RECURSE EXCLUDE_FILES
        ${PROJECT_SOURCE_DIR}/sources/ScriptHandler/*.h
        ${PROJECT_SOURCE_DIR}/sources/ScriptHandler/*.cpp
        ${PROJECT_SOURCE_DIR}/sources/ProcessingVideo/*.h
        ${PROJECT_SOURCE_DIR}/sources/ProcessingVideo/*.cpp
        ${PROJECT_SOURCE_DIR}/sources/Overlay/ScriptsOverlay.h
        ${PROJECT_SOURCE_DIR}/sources/Overlay/ScriptsOverlay.cpp
    )

    list(REMOVE_ITEM SOURCE_FILES ${EXCLUDE_FILES})
endif()

set(TARGET_NAME simulator)

setup_main_executable()

target_link_libraries(simulator
    ${OpenCV_LIBS}
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Xml
    ${ZMQ_LIBRARIES}
)

file(GLOB_RECURSE PROJECT_RESOURCES
    ${PROJECT_SOURCE_DIR}/resources/*
)

if(WIN32)
    set(OpenCV_DIR_DLL "${OpenCV_DIR}/bin/$<CONFIGURATION>")
    include(windeployqt)
    windeployqt(simulator)

    set(ALL_DLLs
       ${OpenCV_DIR_DLL}/opencv_highgui430.dll
       ${OpenCV_DIR_DLL}/opencv_imgproc430.dll
       ${OpenCV_DIR_DLL}/opencv_core430.dll
       ${OpenCV_DIR_DLL}/opencv_imgcodecs430.dll
       ${OpenCV_DIR_DLL}/opencv_videoio430.dll
       ${GSTREAMER_DIR}/intl-8.dll
       ${GSTREAMER_DIR}/ffi-7.dll
       ${GSTREAMER_DIR}/z-1.dll
       ${GSTREAMER_DIR}/orc-0.4-0.dll
       ${GSTREAMER_DIR}/gstapp-1.0-0.dll
       ${GSTREAMER_DIR}/gstreamer-1.0-0.dll
       ${GSTREAMER_DIR}/gstriff-1.0-0.dll
       ${GSTREAMER_DIR}/gstpbutils-1.0-0.dll
       ${GSTREAMER_DIR}/gstbase-1.0-0.dll
       ${GSTREAMER_DIR}/gsttag-1.0-0.dll
       ${GSTREAMER_DIR}/gstaudio-1.0-0.dll
       ${GSTREAMER_DIR}/gstvideo-1.0-0.dll
       ${GSTREAMER_DIR}/glib-2.0-0.dll
       ${GSTREAMER_DIR}/gobject-2.0-0.dll
       ${GSTREAMER_DIR}/gmodule-2.0-0.dll
       ${GSTREAMER_DIR}/pcre2-8-0.dll
       ${ZeroMQ_DIR}/bin/Release/libzmq-v142-mt-4_3_5.dll
    )

    foreach( file_i ${ALL_DLLs})
        add_custom_command(TARGET simulator POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${file_i}" $<TARGET_FILE_DIR:simulator>
    )
    endforeach( file_i )
endif()

install(FILES ${PROJECT_SOURCE_DIR}/resources/simulator.pck DESTINATION bin)
