cmake_minimum_required(VERSION 3.16)

#set(Boost_USE_STATIC_LIBS ON)
set(BUILD_SHARED_LIBS OFF)

pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

set(EXE_NAME camerapro)

add_executable(${EXE_NAME})
target_sources(
        ${EXE_NAME}
        PRIVATE
        src/main.cpp
        src/application.cpp
        src/application.hpp
        src/camera.cpp
        src/camera.hpp
        src/webcamera.cpp
        src/webcamera.hpp
        src/camerabuilder.cpp
        src/camerabuilder.hpp
        src/frameDisplay.cpp src/frameDisplay.hpp

        src/cp_config.cpp
        src/cp_ipc_cam2dp1.cpp
        src/cp_ipc_cam2dp1_if.h
        src/cp_config.h

        src/args.cpp
        src/args.hpp
        )
target_include_directories(
        ${EXE_NAME}
        PRIVATE
        ../common
        ..
)

target_link_libraries(
        ${EXE_NAME}
        PRIVATE
        log4cxx
        camera_frame
        model
        PkgConfig::jansson
        opencv_world
        turret
        fmt::fmt
)



#add_custom_command(TARGET camerapro POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        ${CMAKE_CURRENT_SOURCE_DIR}/config
#        ${CMAKE_CURRENT_BINARY_DIR}/config)

add_custom_command(TARGET ${EXE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/camerapro-log.xml
        ${CMAKE_CURRENT_BINARY_DIR}/config/camerapro-log.xml
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config/camerapro-log.xml)

add_custom_command(TARGET ${EXE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/camerapro.json
        ${CMAKE_CURRENT_BINARY_DIR}/config/camerapro.json
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config/camerapro.json)

# Add a custom target that depends on the copy command
#add_custom_target(copy_file ALL DEPENDS ${DESTINATION_DIR}/cam_log.xml)