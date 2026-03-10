cmake_minimum_required(VERSION 3.16)
project(calibration)

set(Boost_USE_STATIC_LIBS ON)

include(FindPkgConfig)

pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

add_executable(${PROJECT_NAME})
target_sources(
        ${PROJECT_NAME}
        PRIVATE
        src/camera_calibration.cpp
)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        $<TARGET_PROPERTY:log4cxx,INTERFACE_INCLUDE_DIRECTORIES>
        src
)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        Boost::headers
        PkgConfig::jansson
        pthread
        log4cxx
        opencv_world
        camera_frame
        model
)