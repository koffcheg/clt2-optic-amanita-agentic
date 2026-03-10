cmake_minimum_required(VERSION 3.16)
project(dp2-data-gen)

set(Boost_USE_STATIC_LIBS ON)

include(FindPkgConfig)

pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

add_executable(${PROJECT_NAME})
target_sources(
        ${PROJECT_NAME}
        PRIVATE
        src/dp2_tdg_main.cpp
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
