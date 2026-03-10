// datapro2 rpc "client"
// Created by u on 15.07.24.
//

#ifndef CLT_OPTIC_DP2_RPC_CL_H
#define CLT_OPTIC_DP2_RPC_CL_H

#include "m_rpc_sink.h"
namespace dp2 {	class session;}

class dp2m_rpc_cl : public rpc_sink {
	void on_rd_msg_complite(const uint8_t *data, std::size_t len) override;

	void on_heart_beat() override;
};

std::unique_ptr<rpc_sink> cr_m_rpc_sink();

#endif //CLT_OPTIC_DP2_RPC_CL_H
