
set(LIB_NAME camera_frame)


add_library(${LIB_NAME} STATIC)


target_include_directories(${LIB_NAME}
                           INTERFACE
                           src)

target_sources(
        ${LIB_NAME}
        PRIVATE
        src/camera_frame.cpp
        PUBLIC
        src/camera_frame.hpp
        )

target_link_libraries(
        ${LIB_NAME}
        PRIVATE
        opencv_world
      )
