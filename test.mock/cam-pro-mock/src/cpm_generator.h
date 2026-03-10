#ifndef CLT_OPTIC_CPM_GENERATOR_H
#define CLT_OPTIC_CPM_GENERATOR_H

namespace cam_pro { class Config; class ipc_data_tr;}

class cam_pro_test_generator {
	cam_pro::ipc_data_tr *data_forwarder;
public:
	explicit cam_pro_test_generator(const cam_pro::Config &);

	void run();
};

#endif //CLT_OPTIC_CPM_GENERATOR_H
