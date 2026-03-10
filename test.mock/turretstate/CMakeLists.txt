cmake_minimum_required(VERSION 3.16)

#set(Boost_USE_STATIC_LIBS ON)
set(BUILD_SHARED_LIBS OFF)

pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

set(EXE_NAME turretstate)

add_executable(${EXE_NAME})

target_sources(
        ${EXE_NAME}
        PRIVATE
        src/main.cpp
        src/application.cpp
        src/application.hpp

        src/args.cpp
        src/args.hpp
        )
target_include_directories(
        ${EXE_NAME}
        PRIVATE
)

target_link_libraries(
        ${EXE_NAME}
        PRIVATE
        log4cxx
        jansson
        fmt::fmt
        turret
)


add_custom_command(TARGET ${EXE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/turretstate-log.xml
        ${CMAKE_CURRENT_BINARY_DIR}/turretstate-log.xml
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config/turretstate-log.xml)
