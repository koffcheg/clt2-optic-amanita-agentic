cmake_minimum_required(VERSION 3.16)
project(datapro1_v2)

include(FindPkgConfig)

if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    find_package(Threads REQUIRED)
    find_package(OpenCV REQUIRED)
    find_package(Boost REQUIRED)
    find_package(log4cxx REQUIRED)

    if (NOT TARGET model)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../common/model ${CMAKE_CURRENT_BINARY_DIR}/common/model)
    endif ()
    if (NOT TARGET camera_frame)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../common/frame ${CMAKE_CURRENT_BINARY_DIR}/common/frame)
    endif ()
endif ()

pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

add_executable(${PROJECT_NAME})

target_sources(
        ${PROJECT_NAME}
        PRIVATE
        src/main.cpp
        src/app/process_control.cpp
        src/app/startup.cpp
        src/config/config.cpp
        src/frame/frame_normalizer.cpp
        src/frame/frame_packet_builder.cpp
        src/frame/frame_context.cpp
        src/result/result_builder.cpp
        src/result/result_sink.cpp
        src/runtime/runtime.cpp
        src/runtime/pipeline.cpp
        src/runtime/cyclic_frame_buffer.cpp
        src/source/source.cpp
        src/source/uri_file_source.cpp
        src/source/ipc_frame_source.cpp
        src/source/source_factory.cpp
        src/stages/calibration.cpp
        src/stages/input_normalization_stage.cpp
        src/stages/prep_stage.cpp
        src/stages/radiometric_stage.inverse_median.cpp
        src/stages/radiometric_stage.cpp
        src/visualization/visualization_sink.cpp
        ../datapro1/src/dp1_tr_res2dp2.cpp
        ../datapro1/src/dp1_rpc_data_mrsh.cpp
        ../datapro1/src/dataproSaveToFile.cpp
        ../common/utils/datetime.cpp
)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        include
        ../datapro1/src
        ../common/model
        ../common/utils
        ../common/frame/src
        ${OpenCV_INCLUDE_DIRS}
)

if (TARGET opencv_world)
    target_link_libraries(
            ${PROJECT_NAME}
            PRIVATE
            opencv_world
            PkgConfig::jansson
    )
else ()
    target_link_libraries(
            ${PROJECT_NAME}
            PRIVATE
            ${OpenCV_LIBS}
            PkgConfig::jansson
    )
endif ()

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        Boost::headers
        Threads::Threads
        log4cxx
        model
        camera_frame
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/application_config_datapro1_v2.json
        ${CMAKE_CURRENT_BINARY_DIR}/config/application_config_datapro1_v2.json
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/pipeline_config_datapro1_v2.json
        ${CMAKE_CURRENT_BINARY_DIR}/config/pipeline_config_datapro1_v2.json
        DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/config/application_config_datapro1_v2.json
        ${CMAKE_CURRENT_SOURCE_DIR}/config/pipeline_config_datapro1_v2.json)

if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    option(DP1V2_BUILD_LOCAL_TESTS "Build datapro1_v2 unit and visual-unit tests in a standalone configure" ON)

    if (DP1V2_BUILD_LOCAL_TESTS)
        enable_testing()
        include(GoogleTest)

        if (NOT TARGET gtest)
            set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
            add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../gtests/lib ${CMAKE_CURRENT_BINARY_DIR}/gtests/lib)
        endif ()

        add_executable(dp1v2_unit_tests)
        target_sources(
                dp1v2_unit_tests
                PRIVATE
                ../tests/unit/dp1/t_dp1v2_cyclic_frame_buffer.cpp
                ../tests/unit/dp1/t_dp1v2_config.cpp
                ../tests/unit/dp1/t_dp1v2_visualization_sink.cpp
                ../tests/unit/dp1/t_dp1v2_source.cpp
                ../tests/unit/dp1/t_dp1v2_frame_context.cpp
                ../tests/unit/dp1/t_dp1v2_input_normalization_stage.cpp
                ../tests/unit/dp1/t_dp1v2_prep_stage.cpp
                ../tests/unit/dp1/t_dp1v2_pipeline.cpp
                src/runtime/cyclic_frame_buffer.cpp
                src/config/config.cpp
                src/frame/frame_packet_builder.cpp
                src/frame/frame_context.cpp
                src/visualization/visualization_sink.cpp
                src/stages/input_normalization_stage.cpp
                src/stages/prep_stage.cpp
                src/stages/radiometric_stage.cpp
                src/result/result_builder.cpp
                src/runtime/pipeline.cpp
                src/stages/radiometric_stage.inverse_median.cpp
                src/source/uri_file_source.cpp
        )
        target_include_directories(
                dp1v2_unit_tests
                PRIVATE
                include
                ../common/model
                ${OpenCV_INCLUDE_DIRS}
                ../datapro1/src
                ../common/utils
        )
        if (TARGET opencv_world)
            target_link_libraries(dp1v2_unit_tests PRIVATE opencv_world)
        else ()
            target_link_libraries(dp1v2_unit_tests PRIVATE ${OpenCV_LIBS})
        endif ()
        target_link_libraries(
                dp1v2_unit_tests
                PRIVATE
                gtest
                gtest_main
                PkgConfig::jansson
        )
        gtest_discover_tests(dp1v2_unit_tests)

        add_executable(dp1v2_visual_unit_tests)
        target_sources(
                dp1v2_visual_unit_tests
                PRIVATE
                ../tests/visual-unit/dp1/t_dp1v2_inverse_median.cpp
                src/runtime/cyclic_frame_buffer.cpp
                src/stages/radiometric_stage.inverse_median.cpp
        )
        target_include_directories(
                dp1v2_visual_unit_tests
                PRIVATE
                include
                ${OpenCV_INCLUDE_DIRS}
        )
        if (TARGET opencv_world)
            target_link_libraries(dp1v2_visual_unit_tests PRIVATE opencv_world)
        else ()
            target_link_libraries(dp1v2_visual_unit_tests PRIVATE ${OpenCV_LIBS})
        endif ()
        target_link_libraries(
                dp1v2_visual_unit_tests
                PRIVATE
                gtest
                gtest_main
        )
        gtest_discover_tests(dp1v2_visual_unit_tests)
    endif ()
endif ()
