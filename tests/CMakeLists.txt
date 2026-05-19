project(policy_tests)

include(GoogleTest)
pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

add_executable(dp1v2_unit_tests)
target_sources(
        dp1v2_unit_tests
        PRIVATE
        unit/dp1/t_dp1v2_cyclic_frame_buffer.cpp
        unit/dp1/t_dp1v2_config.cpp
        unit/dp1/t_dp1v2_visualization_sink.cpp
        unit/dp1/t_dp1v2_source.cpp
        unit/dp1/t_dp1v2_frame_context.cpp
        unit/dp1/t_dp1v2_input_normalization_stage.cpp
        ../datapro1_v2/src/runtime/cyclic_frame_buffer.cpp
        ../datapro1_v2/src/config/config.cpp
        ../datapro1_v2/src/frame/frame_packet_builder.cpp
        ../datapro1_v2/src/frame/frame_context.cpp
        ../datapro1_v2/src/visualization/visualization_sink.cpp
        ../datapro1_v2/src/stages/input_normalization_stage.cpp
        ../datapro1_v2/src/stages/radiometric_stage.inverse_median.cpp
        ../datapro1_v2/src/source/uri_file_source.cpp
)

target_include_directories(
        dp1v2_unit_tests
        PRIVATE
        ../datapro1_v2/include
        ../common/model
)

target_link_libraries(
        dp1v2_unit_tests
        PRIVATE
        gtest
        gtest_main
        opencv_world
        PkgConfig::jansson
)

gtest_discover_tests(dp1v2_unit_tests)

add_executable(dp1v2_visual_unit_tests)
target_sources(
        dp1v2_visual_unit_tests
        PRIVATE
        visual-unit/dp1/t_dp1v2_inverse_median.cpp
        ../datapro1_v2/src/runtime/cyclic_frame_buffer.cpp
        ../datapro1_v2/src/stages/radiometric_stage.inverse_median.cpp
)

target_include_directories(
        dp1v2_visual_unit_tests
        PRIVATE
        ../datapro1_v2/include
)

target_link_libraries(
        dp1v2_visual_unit_tests
        PRIVATE
        gtest
        gtest_main
        opencv_world
)

gtest_discover_tests(dp1v2_visual_unit_tests)
