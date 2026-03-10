cmake_minimum_required(VERSION 3.16)

project(turretgui)

include(FindPkgConfig)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0144 NEW)

option(RUN_CLANG_TIDY "Run clang-tidy" OFF)
if (RUN_CLANG_TIDY)
    find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy-15 clang-tidy-14 clang-tidy-12 REQUIRED)
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
    src/decimalSeparator.cpp src/decimalSeparator.hpp
    src/trackTest.cpp src/trackTest.hpp
    src/application.cpp src/application.hpp
    src/theme.cpp src/theme.hpp
    src/args.cpp src/args.hpp
    src/config.cpp src/config.hpp
    src/wGoto.cpp src/wGoto.hpp
    src/wSpeed.cpp src/wSpeed.hpp
    src/wTrackTest.cpp src/wTrackTest.hpp
    src/wTrack.cpp src/wTrack.hpp
    src/wLeftPanel.cpp src/wLeftPanel.hpp
    src/window.cpp src/window.hpp
    src/tg_rc_from_dp2.cpp src/tg_rc_from_dp2.h
    src/turretControl.cpp src/turretControl.hpp

    ../common/charts/wSkyChart.cpp ../common/charts/wSkyChart.hpp
    ../common/charts/wChart.cpp ../common/charts/wChart.hpp
    ../common/charts/wChartAxes.cpp ../common/charts/wChartAxes.hpp
    ../common/charts/wChartPlot.cpp ../common/charts/wChartPlot.hpp
    ../common/charts/wChartLegend.cpp ../common/charts/wChartLegend.hpp
    ../common/charts/wLed.cpp ../common/charts/wLed.hpp

    ../datapro2/src/dp2_svr.cpp
    ../datapro2/src/dp2_ses.cpp
    ../datapro2/src/dp2_rpc_data_mrsh.cpp
    ../datapro2/src/coordinateTransformation.cpp
       
    ../common/utils/fileLogger.cpp ../common/utils/fileLogger.cpp
)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        src
        ../common
        ..
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Widgets
    fmt::fmt
    Boost::headers
    opencv_world
    jansson
    log4cxx
    turret
    model
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/turretgui-log.xml
        ${CMAKE_CURRENT_BINARY_DIR}/turretgui-log.xml
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config/turretgui-log.xml)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/turretgui.json
        ${CMAKE_CURRENT_BINARY_DIR}/turretgui.json
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config/turretgui.json)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../common/model ${CMAKE_CURRENT_BINARY_DIR}/common/model)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../common/turret ${CMAKE_CURRENT_BINARY_DIR}/common/turret)
