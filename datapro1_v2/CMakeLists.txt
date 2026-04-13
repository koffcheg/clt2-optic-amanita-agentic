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
        ${OpenCV_INCLUDE_DIRS}
)

if (TARGET opencv_world)
    target_link_libraries(
            ${PROJECT_NAME}
            PRIVATE
            opencv_world
    )
else ()
    target_link_libraries(
            ${PROJECT_NAME}
            PRIVATE
            ${OpenCV_LIBS}
    )
endif ()
