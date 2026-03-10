cmake_minimum_required(VERSION 3.16)

project(plotter)

include(FindPkgConfig)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0144 NEW)

option(RUN_CLANG_TIDY "Run clang-tidy" OFF)
if (RUN_CLANG_TIDY)
    find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy-14 clang-tidy-12 REQUIRED)
    message("CLANG_TIDY_EXECUTABLE = ${CLANG_TIDY_EXECUTABLE}")
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXECUTABLE} -p ${CMAKE_BINARY_DIR})
endif ()

#EXPAT lib needs for static version of log4cxx
find_package(EXPAT REQUIRED)
find_package(Threads REQUIRED)
find_package(log4cxx REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL OpenGLWidgets Widgets)

pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

#qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt_add_executable(${PROJECT_NAME}
    src/main.cpp
    resources.qrc
    src/application.cpp src/application.hpp
    src/theme.cpp src/theme.hpp
    src/args.cpp src/args.hpp
    src/config.cpp src/config.hpp

    src/wLeftPanel.cpp src/wLeftPanel.hpp
    src/window.cpp src/window.hpp
    src/fileReader.cpp src/fileReader.hpp
   
#todo: make a library out of the followin
    ../common/charts/wChart.cpp ../common/charts/wChart.hpp
    ../common/charts/wChartAxes.cpp ../common/charts/wChartAxes.hpp
    ../common/charts/wChartPlot.cpp ../common/charts/wChartPlot.hpp
    ../common/charts/wChartLegend.cpp ../common/charts/wChartLegend.hpp

    ../common/utils/decimalSeparator.cpp ../common/utils/decimalSeparator.hpp

)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        src
        ..
        ../common
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Widgets
    fmt::fmt
    Boost::headers
    jansson
    log4cxx
    turret
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/plotter-log.xml
        ${CMAKE_CURRENT_BINARY_DIR}/plotter-log.xml
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config/plotter-log.xml)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/plotter.json
        ${CMAKE_CURRENT_BINARY_DIR}/plotter.json
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config/plotter.json)

#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../common/model ${CMAKE_CURRENT_BINARY_DIR}/common/model)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../common/turret ${CMAKE_CURRENT_BINARY_DIR}/common/turret)
