cmake_minimum_required(VERSION 3.16)
project(datapro1)

set(Boost_USE_STATIC_LIBS ON)

include(FindPkgConfig)

pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

add_executable(${PROJECT_NAME})
target_sources(
        ${PROJECT_NAME}
        PRIVATE
        src/dp1_main.cpp
        src/dp1_config.cpp
        src/dataproFilter.cpp
        src/datarpoFragmentation.cpp
        src/datarpoSegmentation.cpp
        src/dataproSaveToFile.cpp
        src/dataproOtherSource.cpp
        src/datapro1.cpp
        src/dataproContourOperations.cpp
        src/dataproMedianFilter2.cpp
        src/dp1_ipc_runner.cpp
        src/dp1_uri_runner.cpp
        src/dp1_frame_proc.cpp
        src/dp1_ipc.cpp
        src/dp1_tr_res2dp2.cpp
        src/dp1_rpc_data_mrsh.cpp
        src/dp1_calc_limit.cpp
        src/dp1_snail_path.cpp
        src/dataproCameraCalibration.cpp
        ../common/model/m_cfg.cpp
        ../common/utils/ut_signal.cpp
        ../common/utils/datetime.cpp
        )

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        $<TARGET_PROPERTY:log4cxx,INTERFACE_INCLUDE_DIRECTORIES>
        src
        ../common/model
        ../common/frame/src
        ../common/utils
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

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/dp1_log.xml
        ${CMAKE_CURRENT_BINARY_DIR}/config/dp1_log.xml
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config/dp1_log.xml)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/config_datapro1.json
        ${CMAKE_CURRENT_BINARY_DIR}/config/config_datapro1.json
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config/config_datapro1.json)
