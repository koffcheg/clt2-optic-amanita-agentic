#ifndef CLT_OPTIC_DP1_IPC_H
#define CLT_OPTIC_DP1_IPC_H

#include <functional>
#include <memory>
#include "dp1_config.h"
#include "m_ipc_def.h"

namespace ns_datapro1 {
	class ipc_data_rc {
	public:
		virtual ~ipc_data_rc() = default;

		//creates a new thread for receiving data
		//on receiving new frame it will bw copy and supplied callback will be invoked (in receiving thread)
		virtual void
		receive_data(std::function<void(ipc_rc_data_store, std::size_t frame_size, std::size_t frame_ipc_index, ipc_time_point)>) = 0;
	};

	std::unique_ptr<ipc_data_rc> get_ipc_data_receiver(const prg_config &cfg, bool (*need_stop)());

	void init_ipc_logger(int cam_index);

}

#endif //CLT_OPTIC_DP1_IPC_H
