#include "m_cfg_if.h"

ipc_name_cfg::ipc_name_cfg(int camera_index) : camera_index_{camera_index} {
	shmem_obj_name_ = "/clt_optic_cam-dp1_" + std::to_string(camera_index_);
	queue_obj_name_ = "/clt_optic_cam_q_" + std::to_string(camera_index_);
}
