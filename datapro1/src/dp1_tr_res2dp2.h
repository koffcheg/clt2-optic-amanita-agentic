//
// Created by user on 23.07.24.
//

#ifndef CLT_OPTIC_DP1_TR_RES2DP2_H
#define CLT_OPTIC_DP1_TR_RES2DP2_H

#include <cstdint>
#include <string>

struct TDataRes;
struct TDataCalibrationCamera;
namespace ns_datapro1 {
	void send_res_to_dp2(const TDataRes &data);

	void init_connect_to_dp2(const std::string &dp2_host, uint16_t port, int reconn_interv_sec, int cam_index, const TDataCalibrationCamera&);

	void dp1_prc_check_tr_hb();    //check transmit heart-beta
}
#endif //CLT_OPTIC_DP1_TR_RES2DP2_H
