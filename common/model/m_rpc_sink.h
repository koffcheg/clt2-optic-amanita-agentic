#ifndef CLT_OPTIC_M_RPC_SINK_H
#define CLT_OPTIC_M_RPC_SINK_H

#include <cstddef>
#include <cstdint>
#include <vector>

class rpc_sink{
public:
	void on_next_raw_read(const uint8_t *data, std::size_t len, bool &break_conn);
	virtual void on_rd_msg_complite(const uint8_t *data, std::size_t len) = 0;
	virtual void on_heart_beat(){}
	virtual ~rpc_sink() = default;
private:
	enum en_rd_stage{
		rd_st_msg_index,
		rd_st_msg_type,
		rd_st_read_len,
		rd_st_read_body,
	};
	size_t read_next_stage(const uint8_t *data, std::size_t len, bool &break_conn);
	en_rd_stage rd_stage{rd_st_msg_index};
	uint8_t msg_type{};
	uint32_t msg_index{};
	size_t msg_len{};
	std::vector<uint8_t> rd_data{std::vector<uint8_t>(1024, 0)};
	std::size_t stage_num_read{};
	enum aux_data{
		aux_max_msg_len = 100*1024*1024,
	};
	size_t read_stage_msg_index(const uint8_t *data, std::size_t len, bool &break_conn);
	size_t read_stage_msg_type(const uint8_t *data, std::size_t len);
	size_t read_stage_msg_len(const uint8_t *data, std::size_t len, bool &break_conn);
	size_t read_stage_msg_body(const uint8_t *data, std::size_t len);
};
#endif //CLT_OPTIC_M_RPC_SINK_H
