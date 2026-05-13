cmake_minimum_required(VERSION 3.16)

project(clt-optic)

include(FindPkgConfig)

# compile commands json for vim completion
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0144 NEW)

option(RUN_CLANG_TIDY "Run clang-tidy" OFF)
if (RUN_CLANG_TIDY)
    find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy clang-tidy-15 clang-tidy-14 clang-tidy-12 REQUIRED)
    message("CLANG_TIDY_EXECUTABLE = ${CLANG_TIDY_EXECUTABLE}")
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXECUTABLE} -p ${CMAKE_BINARY_DIR})
endif ()

find_package(Threads REQUIRED)

#EXPAT lib needs for static version of log4cxx
find_package(EXPAT REQUIRED)
find_package(log4cxx REQUIRED)

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)
find_package(Clipper2 REQUIRED)
find_package(fmt REQUIRED)

add_subdirectory(common/frame)
add_subdirectory(common/model)
add_subdirectory(common/turret)
add_subdirectory(camerapro)
add_subdirectory(datapro1)
add_subdirectory(datapro1_v2)
add_subdirectory(datapro2)
add_subdirectory(turretpro)
add_subdirectory(gtests)
add_subdirectory(tests)
add_subdirectory(test.mock/cam-pro-mock)
add_subdirectory(test.mock/datapro1-mock)
add_subdirectory(test.mock/datapro2-mock)
add_subdirectory(test.mock/dp2-data-gen)
add_subdirectory(test.mock/turretstate)
add_subdirectory(manager)
add_subdirectory(viewer)
add_subdirectory(calibration)
