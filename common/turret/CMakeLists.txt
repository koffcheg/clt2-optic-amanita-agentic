
set(LIB_NAME turret)


add_library(${LIB_NAME} STATIC)


target_include_directories(${LIB_NAME}
                           INTERFACE
                           .)

target_sources(
        ${LIB_NAME}
        PRIVATE
        turretState.cpp
        turretCommands.cpp
        slip.cpp
        serializer.cpp
        sockUtils.cpp
        turretInfoClient.cpp
        turretControlClient.cpp
        tcpClient.cpp
        PUBLIC
        turretTypes.hpp
        turretState.hpp
        turretCommands.hpp
        slip.hpp
        sockUtils.hpp
        serializer.hpp
        tcpClient.hpp
        turretInfoClient.hpp
        turretControlClient.hpp
        utils.hpp
        )

#target_link_libraries(
#        ${LIB_NAME}
#        PRIVATE
#        opencv_world
#      )
