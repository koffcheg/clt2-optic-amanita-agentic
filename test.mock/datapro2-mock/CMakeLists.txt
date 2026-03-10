cmake_minimum_required(VERSION 3.16)
project(datapro2-mock)

set(Boost_USE_STATIC_LIBS ON)

include(FindPkgConfig)

pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

add_executable(${PROJECT_NAME})
target_sources(
        ${PROJECT_NAME}
        PRIVATE
        src/dp2m_main.cpp
        src/dp2m_rpc_cl.cpp
        ../../datapro2/src/dp2_ses.cpp
        ../../datapro2/src/dp2_svr.cpp
        ../../datapro2/src/dp2_cfg.cpp
        )

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        src
        ../../datapro2/src
        ../../common/model
)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        Boost::headers
        log4cxx
        PkgConfig::jansson
        pthread
        model
)
