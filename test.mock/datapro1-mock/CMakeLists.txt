cmake_minimum_required(VERSION 3.16)
project(datapro1-mock)

set(Boost_USE_STATIC_LIBS ON)

include(FindPkgConfig)

pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

add_executable(${PROJECT_NAME})
target_sources(
        ${PROJECT_NAME}
        PRIVATE
        src/dp1m_main.cpp
        src/dp1m_receiver.cpp
        ../../common/model/m_cfg.cpp
        ../../common/utils/ut_signal.cpp
        ../../datapro1/src/dp1_ipc.cpp
        ../../datapro1/src/dp1_config.cpp

)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        $<TARGET_PROPERTY:log4cxx,INTERFACE_INCLUDE_DIRECTORIES>
        src
        ../../datapro1/src
        ../../common/model
        ../../common/utils
)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        rt
        PkgConfig::jansson
        pthread
        log4cxx
)
