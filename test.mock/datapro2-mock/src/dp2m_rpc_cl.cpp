#include <log4cxx/logger.h>
#include "dp2m_rpc_cl.h"

static auto logger = log4cxx::Logger::getLogger("rpc-m-sink");

void dp2m_rpc_cl::on_rd_msg_complite([[maybe_unused]] const uint8_t *data, std::size_t len){
	LOG4CXX_DEBUG(logger, "rc new msg, len: " << len);
}

void dp2m_rpc_cl::on_heart_beat(){
	LOG4CXX_DEBUG(logger, "heart - beat");
}

std::unique_ptr<rpc_sink> cr_m_rpc_sink(){
	return std::make_unique<dp2m_rpc_cl>();
}
