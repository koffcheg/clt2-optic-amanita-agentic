#ifndef CLT_OPTIC_M_CFG_IF_H
#define CLT_OPTIC_M_CFG_IF_H

#include <string>

class ipc_name_cfg {
	int camera_index_;
	std::string shmem_obj_name_;
	std::string queue_obj_name_;
public:
	explicit ipc_name_cfg(int camera_index);

	[[nodiscard]] const char *get_shmem_obj_name() const { return shmem_obj_name_.c_str(); };

	[[nodiscard]] const char *get_queue_obj_name() const { return queue_obj_name_.c_str(); };
};

#endif //CLT_OPTIC_M_CFG_IF_H
