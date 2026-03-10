// datapro2 rpc dataformer
// Created by u on 15.07.24.
//

#ifndef CLT_OPTIC_M_DP2_RPC_D_FORMER_H
#define CLT_OPTIC_M_DP2_RPC_D_FORMER_H

#include <cstdint>
#include <vector>
#include <chrono>
#include <span>

class rpc_data_former{
public:
	std::span<uint8_t> form_next_msg(const uint8_t *payload_data, std::size_t payload_len);

	//if need (time to ) then return  formed message (with heart-beat)
	//if it's too early, return empty span
	std::span<uint8_t> check_heart_beat(bool force_form_hb);
	void reset();
private:
	std::vector<uint8_t> formed_data;
	uint32_t msg_index{};
	std::chrono::time_point<std::chrono::steady_clock> last_hb_time{};
};
#endif //CLT_OPTIC_M_DP2_RPC_D_FORMER_H
