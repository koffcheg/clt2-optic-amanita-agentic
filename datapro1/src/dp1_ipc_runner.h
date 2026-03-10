#ifndef CLT_OPTIC_DP1_RUNNER_H
#define CLT_OPTIC_DP1_RUNNER_H

struct TDataCalibrationCamera;
namespace ns_datapro1 {
	class prg_config;

	void init_ipc_runner_logger(int cam_index);

	int run_ipc_src(const prg_config &cgf, const TDataCalibrationCamera &cam_cfg, const int& cam_index, bool (*need_stop)());
}

#endif //CLT_OPTIC_DP1_RUNNER_H
