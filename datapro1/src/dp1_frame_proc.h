#ifndef CLT_OPTIC_DP1_FRAME_PROC_H
#define CLT_OPTIC_DP1_FRAME_PROC_H

#include <cstddef>
#include <memory>
#include "m_ipc_def.h"

struct TDataCalibrationCamera;
namespace cv{class Mat;}
namespace cam_pro{struct FrameHeader;}

namespace ns_datapro1 {
	class prg_config;
	struct frame_n_header
	{
		cv::Mat *mat;
		const cam_pro::FrameHeader *cam_pro_header;
		ipc_time_point ipc_start_time;
	};

	class frame_processor {
	public:
		virtual ~frame_processor() = default;
		virtual void proc_next_frame(frame_n_header rc_frame) = 0;
	};

	std::unique_ptr<frame_processor> get_fr_processor(const prg_config &cfg, const TDataCalibrationCamera &cam_cfg, int frame_width, int frame_height, int cam_index);
	void init_fr_proc_logger(int cam_index);
}
#endif //CLT_OPTIC_DP1_FRAME_PROC_H
