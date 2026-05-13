project(policy_tests)

include(GoogleTest)

add_executable(dp1v2_unit_tests)
target_sources(
        dp1v2_unit_tests
        PRIVATE
        unit/dp1/t_dp1v2_cyclic_frame_buffer.cpp
        ../datapro1_v2/src/runtime/cyclic_frame_buffer.cpp
)

target_include_directories(
        dp1v2_unit_tests
        PRIVATE
        ../datapro1_v2/include
)

target_link_libraries(
        dp1v2_unit_tests
        PRIVATE
        gtest
        gtest_main
        opencv_world
)

gtest_discover_tests(dp1v2_unit_tests)

add_executable(dp1v2_visual_unit_tests)
target_sources(
        dp1v2_visual_unit_tests
        PRIVATE
        visual-unit/dp1/t_dp1v2_inverse_median.cpp
        ../datapro1_v2/src/runtime/cyclic_frame_buffer.cpp
        ../datapro1_v2/src/stages/inverse_median.cpp
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
