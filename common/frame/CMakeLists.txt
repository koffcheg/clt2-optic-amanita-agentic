
set(LIB_NAME camera_frame)


add_library(${LIB_NAME} STATIC)


target_include_directories(${LIB_NAME}
                           PUBLIC
                           src
                           ${OpenCV_INCLUDE_DIRS})

target_sources(
        ${LIB_NAME}
        PRIVATE
        src/camera_frame.cpp
        PUBLIC
        src/camera_frame.hpp
        )

if (TARGET opencv_world)
    target_link_libraries(
            ${LIB_NAME}
            PUBLIC
            opencv_world
          )
else ()
    target_link_libraries(
            ${LIB_NAME}
            PUBLIC
            ${OpenCV_LIBS}
          )
endif ()
