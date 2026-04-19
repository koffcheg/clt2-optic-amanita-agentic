cmake_minimum_required(VERSION 3.16)
project(datapro1_v2)

include(FindPkgConfig)
pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

add_executable(${PROJECT_NAME})

target_sources(
        ${PROJECT_NAME}
        PRIVATE
        src/main.cpp
        src/startup.cpp
        src/config.cpp
)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        include
        ../common/model
        ${OpenCV_INCLUDE_DIRS}
)

if (TARGET opencv_world)
    target_link_libraries(
            ${PROJECT_NAME}
            PRIVATE
            opencv_world
            PkgConfig::jansson
    )
else ()
    target_link_libraries(
            ${PROJECT_NAME}
            PRIVATE
            ${OpenCV_LIBS}
            PkgConfig::jansson
    )
endif ()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/config_datapro1_v2.json
        ${CMAKE_CURRENT_BINARY_DIR}/config/config_datapro1_v2.json
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config/config_datapro1_v2.json)
