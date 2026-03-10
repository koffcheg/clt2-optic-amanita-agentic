#include "m_rpc_sink.h"
#include <log4cxx/logger.h>
#include <cstring>

static auto logger = log4cxx::Logger::getLogger("rpc-sink");

void rpc_sink::on_next_raw_read(const uint8_t *data, std::size_t len, bool &break_conn) {
	while (len) {
		auto num_proc = read_next_stage(data, len, break_conn);
		if(break_conn)
			break;
		if (num_proc >= len)
			break;
		len -= num_proc;
		data += num_proc;
	}
}

size_t rpc_sink::read_stage_msg_index(const uint8_t *data, std::size_t len, bool &break_conn) {
	size_t num_consumed{0};
	while (len && stage_num_read < 4) {
		rd_data[stage_num_read++] = *data;
		++data;
		--len;
		++num_consumed;
	}
	if (stage_num_read == 4) {
		const auto *msg_index_ptr = reinterpret_cast<const uint32_t *>(&rd_data[0]);
		auto rd_msg_index = *msg_index_ptr;
		++msg_index;
		if (rd_msg_index != msg_index) {
			break_conn = true;
			LOG4CXX_ERROR(logger,
						  "unmatched rc msg-indexes, expected: " << msg_index << ", received: " << rd_msg_index);
		}
		rd_stage = rd_st_msg_type;
	}
	return num_consumed;
}

size_t rpc_sink::read_stage_msg_type(const uint8_t *data, [[maybe_unused]] std::size_t len) {
	msg_type = *data;
	if (msg_type == 0) {    //heart beat
		on_heart_beat();
		rd_stage = rd_st_msg_index;
	} else
		rd_stage = rd_st_read_len;
	stage_num_read = 0;
	return 1;
}

size_t rpc_sink::read_stage_msg_len(const uint8_t *data, std::size_t len, bool &break_conn) {
	size_t num_consumed{0};
	while (len && stage_num_read < 4) {
		rd_data[stage_num_read++] = *data;
		++data;
		--len;
		++num_consumed;
	}
	if (stage_num_read == 4) {
		const auto *msg_len_ptr = reinterpret_cast<const uint32_t *>(&rd_data[0]);
		msg_len = *msg_len_ptr;
		if (msg_len > aux_max_msg_len)
			break_conn = true;
		else {
			if(msg_len) {
				rd_stage = rd_st_read_body;
				stage_num_read = 0;
				if (rd_data.size() < msg_len)
					rd_data.resize(msg_len);
			}else{
				on_rd_msg_complite(&rd_data[0], 0);
				stage_num_read = 0;
				rd_stage = rd_st_msg_index;
			}
		}
	}
	return num_consumed;
}

size_t rpc_sink::read_stage_msg_body(const uint8_t *data, std::size_t len) {
	size_t need_bytes_to_full_msg = msg_len - stage_num_read;
	size_t can_read = std::min(len, need_bytes_to_full_msg);
	uint8_t *msg_rd_data = &rd_data[stage_num_read];
	memcpy(msg_rd_data, data, can_read);
	stage_num_read += can_read;
	if (stage_num_read == msg_len) {
		on_rd_msg_complite(&rd_data[0], msg_len);
		stage_num_read = 0;
		rd_stage = rd_st_msg_index;
	}
	return can_read;
}

size_t rpc_sink::read_next_stage(const uint8_t *data, std::size_t len, bool &break_conn) {
	break_conn = false;
	switch (rd_stage) {
		case rd_st_msg_index:
			return read_stage_msg_index(data, len, break_conn);
		case rd_st_msg_type:
			return read_stage_msg_type(data, len);
		case rd_st_read_len:
			return read_stage_msg_len(data, len, break_conn);
		case rd_st_read_body:
			return read_stage_msg_body(data, len);
		default:
			break;
	}
	LOG4CXX_ERROR(logger, "undefined reader state: " << rd_stage);
	break_conn = true;
	return 0;
}