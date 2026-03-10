
set(LIB_NAME model)


add_library(${LIB_NAME} STATIC)

target_sources(
        ${LIB_NAME}
        PRIVATE
        m_cfg.cpp
        m_rpc_sink.cpp
        m_rpc_d_former.cpp
        mem_store.cpp
        INTERFACE
        m_cfg_if.h
        m_ipc_def.h
        )

target_include_directories(${LIB_NAME} INTERFACE .)

target_link_libraries(
        ${LIB_NAME}
        PRIVATE
        log4cxx
)
