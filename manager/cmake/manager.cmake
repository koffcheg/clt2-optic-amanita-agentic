set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_NO_BOOST_CMAKE 1)
set(BOOST_VERSION 1.85.0)
set(Boost_NO_WARN_NEW_VERSIONS 1)

find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS system program_options log)

if(Boost_FOUND) 
    message("Boost version is: ${Boost_VERSION}")
    include_directories(${Boost_INCLUDE_DIR})
    link_directories(${Boost_LIBRARY_DIR})
    add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)
endif()

set (MANAGER_DIR ${ROOT_SRC}/manager)
file(GLOB_RECURSE MANAGER_SOURCES "${MANAGER_DIR}/*.cpp")

set (MANAGER_INCLUDE_DIRS ${MANAGER_DIR}/ ${MANAGER_DIR}/actions/ ${MANAGER_DIR}/agent/  ${MANAGER_DIR}/app/  ${MANAGER_DIR}/config/  ${MANAGER_DIR}/process/  ${MANAGER_DIR}/utils/  ${MANAGER_DIR}/worker/ )

add_executable(manager ${MANAGER_SOURCES})
target_include_directories(manager PRIVATE ${MANAGER_INCLUDE_DIRS})
target_link_libraries(manager ${Boost_LIBRARIES} )

set (DUMMY_DIR ${ROOT_SRC}/dummy)
file(GLOB_RECURSE DUMMY_SOURCES "${DUMMY_DIR}/*.cpp")
set(DUMMY_SOURCES ${DUMMY_SOURCES} "${MANAGER_DIR}/config/CLIOptions.cpp")
set (DUMMY_INCLUDE_DIRS ${DUMMY_DIR}/)

add_executable(dummy ${DUMMY_SOURCES})
target_include_directories(dummy PRIVATE ${MANAGER_INCLUDE_DIRS})


# add_executable(manager_cli ${MANAGER_SOURCES})
# target_include_directories(manager_cli PRIVATE ${MANAGER_INCLUDE_DIRS})
# target_link_libraries(manager_cli ${Boost_LIBRARIES} )
# target_compile_definitions(manager_cli PUBLIC MANAGER_CLI=1)




