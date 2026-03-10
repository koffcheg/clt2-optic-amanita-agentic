// datapro2 rpc "client"
// Created by u on 15.07.24.
//

#ifndef CLT_OPTIC_DP2_RPC_CL_H
#define CLT_OPTIC_DP2_RPC_CL_H

#include "m_rpc_sink.h"
#include "datapro2Types.h"

struct dp2strobe_mth_cfg;
//struct data_camera_cfg;
struct binocular_cfg;

class dp2_rpc_cl : public rpc_sink {
	void on_rd_msg_complite(const uint8_t *data, std::size_t len) override;

	void on_heart_beat() override;
public:
	static std::unique_ptr<rpc_sink> cr_sink();
};


void dp2_init_measure_proc_algo(const dp2strobe_mth_cfg &, const binocular_cfg&);

std::vector<Trajectory> get_presented_trajectories();

#endif //CLT_OPTIC_DP2_RPC_CL_H
