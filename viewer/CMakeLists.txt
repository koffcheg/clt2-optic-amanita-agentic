cmake_minimum_required(VERSION 3.16)
project(viewer)

set(Boost_USE_STATIC_LIBS ON)

include(FindPkgConfig)

pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

add_executable(${PROJECT_NAME})
target_sources(
        ${PROJECT_NAME}
        PRIVATE
        src/viewer_main.cpp
        src/viewer_function.cpp
        ../common/model/m_cfg.cpp
        ../common/utils/ut_signal.cpp
        ../common/utils/datetime.cpp
        ../datapro2/src/dataproReadFromFile.cpp
        ../datapro2/src/coordinateTransformation.cpp
        ../datapro2/src/calculationTrajectory.cpp
)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        $<TARGET_PROPERTY:log4cxx,INTERFACE_INCLUDE_DIRECTORIES>
        src
        ../datapro2/src
        ../common/model
        ../common/frame/src
        ../common/utils
        ..
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