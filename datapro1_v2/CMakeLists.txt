cmake_minimum_required(VERSION 3.16)
project(datapro1_v2)

add_executable(${PROJECT_NAME})

target_sources(
        ${PROJECT_NAME}
        PRIVATE
        src/main.cpp
)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        include
)
