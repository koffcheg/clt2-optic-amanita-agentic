# 'Google_test' is the subproject name
project(g_utests)

# 'lib' is the folder with Google Test sources
add_subdirectory(lib)
include_directories(SYSTEM ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

add_executable(${PROJECT_NAME} EXCLUDE_FROM_ALL)
target_sources(
        ${PROJECT_NAME}
        PRIVATE
        src/t_slip.cpp
        src/t_turretState.cpp
        src/t_turretInfoServer.cpp
        src/t_turretControlServer.cpp
        src/t_turretCommands.cpp
        src/t_sitech.cpp
        src/t_config.cpp
        src/t_ipc.cpp
        src/t_clipper_export_headers.cpp
        src/t_rpc.cpp
        src/t_tile_proc_limit.cpp
        src/t_dp2_merge.cpp
        ../common/model/m_cfg.cpp
        ../common/model/m_rpc_sink.cpp
        ../common/model/m_rpc_d_former.cpp
        ../common/model/mem_store.cpp
        ../common/utils/datetime.cpp
        ../datapro1/src/dp1_rpc_data_mrsh.cpp
        ../datapro1/src/dp1_snail_path.cpp
        ../datapro1/src/dp1_calc_limit.cpp
        ../datapro2/src/dp2_rpc_data_mrsh.cpp
        ../datapro2/src/datapro2.cpp
        ../datapro2/src/calculationTrajectory.cpp
        ../datapro2/src/dataproSaveToFile.cpp
        ../datapro2/src/coordinateTransformation.cpp
        ../turretpro/src/infoServer.cpp
        ../turretpro/src/controlServer.cpp
        ../turretpro/src/tcpServer.cpp
        ../turretpro/src/sitech.cpp
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        src
        ../common/model
        ../common/utils
        ../turretpro/src
        ../datapro1/src
        ..
        $<TARGET_PROPERTY:Clipper2::Clipper2,INTERFACE_INCLUDE_DIRECTORIES>
        ..
)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        gtest
        gtest_main
        log4cxx
        turret
        opencv_world
        Clipper2::Clipper2
        fmt::fmt
        PkgConfig::jansson
)
