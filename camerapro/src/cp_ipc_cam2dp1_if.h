#ifndef CLT_OPTIC_CP_IPC_CAM2DP1_IF_H
#define CLT_OPTIC_CP_IPC_CAM2DP1_IF_H

#include <cstddef>
#include "cp_config.h"

namespace cam_pro {
	class ipc_data_tr {
	public:
		virtual ~ipc_data_tr() = default;

		//запит пам'яті для розміщення наступного кадру
		virtual void *get_prt_next_frame() = 0;

		//формування кадру закінчено, передача його на DP1
		virtual bool tr_formed_frame() = 0;

		virtual std::size_t frame_size() = 0;
	};

	ipc_data_tr *get_ipc_data_forwarder(std::size_t frame_size, const Config &cfg);
}
#endif //CLT_OPTIC_CP_IPC_CAM2DP1_IF_H
