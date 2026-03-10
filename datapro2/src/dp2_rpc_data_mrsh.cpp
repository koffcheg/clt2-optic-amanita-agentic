#include "dp2_rpc_data_mrsh.h"
#include "datapro2Types.h"
#include "mem_store.h"
#include <log4cxx/logger.h>

static auto logger = log4cxx::Logger::getLogger("dp2-data-marsh");

void serialize_dp2_res(CMemStore &ms, const Trajectory& data){
	//measurements
	ms.write_native<uint32_t>(data.measurements.size());
	ms.write_native<uint32_t>(sizeof(Measurement));
	for(const auto &el : data.measurements)
		ms.write_native(el);

	//direct members
	ms.write_native(data.id);
	ms.write_native(data.cam_index);
	ms.write_native(data.time0);
	ms.write_native(data.x0_hat);
	ms.write_native(data.y0_hat);
	ms.write_native(data.z0_hat);
	ms.write_native(data.Vx_hat);
	ms.write_native(data.Vy_hat);
	ms.write_native(data.Vz_hat);
	ms.write_native(data.unconfermed_frames_count);
	ms.write_native(data.updated);
	ms.write_native(data.speed);
	ms.write_native(data.acceleration);
	ms.write_native(data.std_dev_x);
	ms.write_native(data.std_dev_y);
	ms.write_native(data.std_dev_z);
	ms.write_native(data.std_dev_xyz);
}

bool deserialize_dp2_res(CMemStore &ms, Trajectory& track){
	//measurements
	uint32_t num_meas;
	ms.read_native(num_meas);
	uint32_t struct_size;
	ms.read_native(struct_size);
	if (struct_size != sizeof(Measurement)) {
		LOG4CXX_ERROR(logger, "Measurement size mismatch, rc: " << struct_size << ", real: " << sizeof(Measurement));
		return false;
	}
	track.measurements.clear();
	Measurement next_meas{};
	for(uint32_t i = 0; i< num_meas; ++i){
		ms.read_native(next_meas);
		track.measurements.push_back(next_meas);
	}

	//direct members
	ms.read_native(track.id);
	ms.read_native(track.cam_index);
	ms.read_native(track.time0);
	ms.read_native(track.x0_hat);
	ms.read_native(track.y0_hat);
	ms.read_native(track.z0_hat);
	ms.read_native(track.Vx_hat);
	ms.read_native(track.Vy_hat);
	ms.read_native(track.Vz_hat);
	ms.read_native(track.unconfermed_frames_count);
	ms.read_native(track.updated);
	ms.read_native(track.speed);
	ms.read_native(track.acceleration);
	ms.read_native(track.std_dev_x);
	ms.read_native(track.std_dev_y);
	ms.read_native(track.std_dev_z);
	ms.read_native(track.std_dev_xyz);
	return true;
}
