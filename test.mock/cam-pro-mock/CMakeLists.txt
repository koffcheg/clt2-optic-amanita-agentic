cmake_minimum_required(VERSION 3.16)
project(camerapro-mock)

set(Boost_USE_STATIC_LIBS ON)

include(FindPkgConfig)

pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

add_executable(${PROJECT_NAME})
target_sources(
        ${PROJECT_NAME}
        PRIVATE
        src/cpm_main.cpp
        src/cpm_generator.cpp
        ../../camerapro/src/cp_config.cpp
        ../../camerapro/src/cp_ipc_cam2dp1.cpp
        ../../common/model/m_cfg.cpp

)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        $<TARGET_PROPERTY:log4cxx,INTERFACE_INCLUDE_DIRECTORIES>
        src
        ../../camerapro/src
        ../../common/model
)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        rt
        PkgConfig::jansson
        pthread
        log4cxx
)
