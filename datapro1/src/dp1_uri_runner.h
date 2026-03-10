#ifndef CLT_OPTIC_DP1_URI_RUNNER_H
#define CLT_OPTIC_DP1_URI_RUNNER_H

struct TDataCalibrationCamera;
namespace ns_datapro1 {
	class prg_config;

	void init_uri_runner_logger(int cam_index);

	int run_uri_src(const prg_config &cgf, const TDataCalibrationCamera &cam_cfg, int cam_index, bool (*need_stop)());
}

#endif //CLT_OPTIC_DP1_URI_RUNNER_H
