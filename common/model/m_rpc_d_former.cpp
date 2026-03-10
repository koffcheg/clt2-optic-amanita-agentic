#include <cstring>
#include "m_rpc_d_former.h"

const size_t aux_msg_header_len = 9;
const size_t aux_hb_header_len = 5;
const uint8_t hb_msg_type = 0;
const uint8_t data_msg_type = 1;

const std::chrono::milliseconds heart_beat_interval{30000};

std::span<uint8_t> rpc_data_former::form_next_msg(const uint8_t *payload_data, std::size_t payload_len) {
	size_t full_masg_len = aux_msg_header_len + payload_len;
	if(formed_data.size() < full_masg_len)
		formed_data.resize(full_masg_len);

	auto *u32p = reinterpret_cast<uint32_t *>(&formed_data[0]);
	*u32p = ++msg_index;

	formed_data[4] = data_msg_type;

	u32p = reinterpret_cast<uint32_t *>(&formed_data[5]);
	*u32p = payload_len;

	//add payload
	memcpy(&formed_data[aux_msg_header_len], payload_data, payload_len);

	return {formed_data.data(), full_masg_len};
}

std::span<uint8_t> rpc_data_former::check_heart_beat(bool force_form_hb){
	auto now_time = std::chrono::steady_clock::now();
	auto time_since_last_hb = now_time - last_hb_time;
	if(!force_form_hb &&  time_since_last_hb < heart_beat_interval)
			return {};

	last_hb_time = now_time;

	if(formed_data.size() < aux_hb_header_len)
		formed_data.resize(aux_hb_header_len);

	auto *u32p = reinterpret_cast<uint32_t *>(&formed_data[0]);
	*u32p = ++msg_index;

	formed_data[4] = hb_msg_type;

	return {formed_data.data(), aux_hb_header_len};
}

void rpc_data_former::reset(){
	msg_index = 0;
	last_hb_time = std::chrono::steady_clock::now();
}


