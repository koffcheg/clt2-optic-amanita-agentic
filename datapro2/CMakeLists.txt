cmake_minimum_required(VERSION 3.16)
project(datapro2)

set(Boost_USE_STATIC_LIBS ON)

include(FindPkgConfig)

pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

add_executable(${PROJECT_NAME})
target_sources(
        ${PROJECT_NAME}
        PRIVATE
        src/dp2_main.cpp
        src/dp2_ses.cpp
        src/dp2_svr.cpp
        src/dp2_cfg.cpp
        src/dp2_rpc_cl.cpp
        src/datapro2.cpp
        src/dataproSaveToFile.cpp
        src/dp2_rpc_data_mrsh.cpp
        src/dp2_tr_to_turret.cpp
        src/dataproReadFromFile.cpp
        src/coordinateTransformation.cpp
        src/calculationTrajectory.cpp
        src/datapro2MultiVision.cpp
        ../datapro1/src/dataproCameraCalibration.cpp
        ../datapro1/src/dp1_rpc_data_mrsh.cpp
        ../common/utils/datetime.cpp
        )

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        src
        #${OpenCV_INCLUDE_DIRS}
        ../common/utils
        ..
)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        PkgConfig::jansson
        pthread
        Boost::headers
        log4cxx
        opencv_world
        model
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/dp2_log.xml
        ${CMAKE_CURRENT_BINARY_DIR}/config/dp2_log.xml
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config/dp2_log.xml)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/config_datapro2.json
        ${CMAKE_CURRENT_BINARY_DIR}/config/config_datapro2.json
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config/config_datapro2.json)
