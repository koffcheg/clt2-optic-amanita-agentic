cmake_minimum_required(VERSION 3.16)

#set(Boost_USE_STATIC_LIBS ON)
set(BUILD_SHARED_LIBS OFF)

pkg_check_modules(jansson REQUIRED IMPORTED_TARGET jansson)

set(EXE_NAME turretpro)

add_executable(${EXE_NAME})

target_sources(
        ${EXE_NAME}
        PRIVATE
        src/main.cpp
        src/application.cpp src/application.hpp
        src/config.cpp src/config.hpp
        src/args.cpp   src/args.hpp
        src/turret.cpp src/turret.hpp
        src/turretSim.cpp src/turretSim.hpp        
        src/turretSitech.cpp src/turretSitech.hpp
        src/pid.cpp src/pid.hpp
        src/infoServer.cpp src/infoServer.hpp
        src/turretInfoServer.hpp
        src/tcpServer.cpp src/tcpServer.hpp
        src/controlServer.cpp src/controlServer.hpp
        src/sharedQueue.hpp
        src/commandsQueue.hpp
        src/turretBuilder.cpp src/turretBuilder.hpp
        src/ceSerial.cpp src/ceSerial.h
        src/sitech.cpp src/sitech.hpp
        ../common/utils/fileLogger.cpp ../common/utils/fileLogger.hpp
        )
target_include_directories(
        ${EXE_NAME}
        PRIVATE
        ../common
)

target_link_libraries(
        ${EXE_NAME}
        PRIVATE
        log4cxx
        jansson
        fmt::fmt
        turret
        model
)



#add_custom_command(TARGET camerapro POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        ${CMAKE_CURRENT_SOURCE_DIR}/config
#        ${CMAKE_CURRENT_BINARY_DIR}/config)

add_custom_command(TARGET ${EXE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/turretpro-log.xml
        ${CMAKE_CURRENT_BINARY_DIR}/config/turretpro-log.xml
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config/turretpro-log.xml)

add_custom_command(TARGET ${EXE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config/turretpro.json
        ${CMAKE_CURRENT_BINARY_DIR}/config/turretpro.json
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config/turretpro.json)

# Add a custom target that depends on the copy command
#add_custom_target(copy_file ALL DEPENDS ${DESTINATION_DIR}/cam_log.xml)