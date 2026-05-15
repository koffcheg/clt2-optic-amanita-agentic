cmake_minimum_required(VERSION 3.16)
project(datapro1_v2)

include(FindPkgConfig)
pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

add_executable(${PROJECT_NAME})

target_sources(
        ${PROJECT_NAME}
        PRIVATE
        src/main.cpp
        src/app/process_control.cpp
        src/app/startup.cpp
        src/config/config.cpp
        src/frame/frame_normalizer.cpp
        src/frame/frame_context.cpp
        src/result/result_builder.cpp
        src/result/result_sink.cpp
        src/runtime/runtime.cpp
        src/runtime/pipeline.cpp
        src/runtime/cyclic_frame_buffer.cpp
        src/source/source.cpp
        src/source/uri_file_source.cpp
        src/source/ipc_frame_source.cpp
        src/source/source_factory.cpp
        src/stages/calibration.cpp
        src/stages/inverse_median.cpp
        ../datapro1/src/dp1_tr_res2dp2.cpp
        ../datapro1/src/dp1_rpc_data_mrsh.cpp
        ../datapro1/src/dataproSaveToFile.cpp
        ../common/utils/datetime.cpp
)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        include
        ../datapro1/src
        ../common/model
        ../common/utils
        ../common/frame/src
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

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        Boost::headers
        pthread
        log4cxx
        model
        camera_frame
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/application_config_datapro1_v2.json
        ${CMAKE_CURRENT_BINARY_DIR}/config/application_config_datapro1_v2.json
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/pipeline_config_datapro1_v2.json
        ${CMAKE_CURRENT_BINARY_DIR}/config/pipeline_config_datapro1_v2.json
        DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/config/application_config_datapro1_v2.json
        ${CMAKE_CURRENT_SOURCE_DIR}/config/pipeline_config_datapro1_v2.json)
