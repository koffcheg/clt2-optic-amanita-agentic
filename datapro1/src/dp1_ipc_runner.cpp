#include "dp1_ipc_runner.h"
#include "dp1_config.h"
#include <log4cxx/logger.h>
#include <condition_variable>
#include "dp1_ipc.h"
#include "camera_frame.hpp"
#include "dp1_frame_proc.h"

static log4cxx::LoggerPtr logger;

namespace ns_datapro1 {

	struct rc_ipc_raw_frame {
		ipc_rc_data_store frame;
		size_t frame_size{};
		size_t frame_index{};
		ipc_time_point ipc_start_time;
		cv::Mat mat;
		cam_pro::Frame *cam_pro_frame{}; //just pointer eq. frame.get()
	};

	static rc_ipc_raw_frame just_rc_frames;
	static bool next_frame_ready{false};
	static std::mutex just_rc_frames_mut;
	static std::condition_variable cv;

	void init_ipc_runner_logger(int cam_index) {
		logger = log4cxx::Logger::getLogger("dp1-" + std::to_string(cam_index) + ".ipc-runner");
	}

	//invoke by receiving thread (not main)
	static void on_rc_next_frame(ipc_rc_data_store frame, std::size_t frame_size, std::size_t frame_ipc_index, ipc_time_point ipc_start_time) {
		{
			std::lock_guard lk{just_rc_frames_mut};

			just_rc_frames.frame = std::move(frame);
			just_rc_frames.frame_size = frame_size;
			just_rc_frames.frame_index = frame_ipc_index;
			just_rc_frames.ipc_start_time = ipc_start_time;
			just_rc_frames.cam_pro_frame = reinterpret_cast<cam_pro::Frame *>(just_rc_frames.frame.get());
			just_rc_frames.mat = just_rc_frames.cam_pro_frame->asMat();

			next_frame_ready = true;
		}
		cv.notify_one();
	}

	int run_ipc_src(const prg_config &cfg, const TDataCalibrationCamera &cam_cfg, const int& cam_index, bool (*need_stop)()){

		auto receiver = ns_datapro1::get_ipc_data_receiver(cfg, need_stop);
		receiver->receive_data(on_rc_next_frame);

		std::deque<rc_ipc_raw_frame> rc_frames;
		std::unique_ptr<frame_processor> fr_proc;
		while (!need_stop()) {
			{
				std::unique_lock lk(just_rc_frames_mut);
				LOG4CXX_DEBUG(logger, "wait for next frame");
				if(!cv.wait_for(lk, std::chrono::milliseconds(500), [] { return next_frame_ready; })){
					LOG4CXX_DEBUG(logger, "time-out waiting next frame");
					if(need_stop()){
						LOG4CXX_DEBUG(logger, "stop detected");
						break;
					}
					continue;
				}
				rc_frames.push_front(std::move(just_rc_frames));
				next_frame_ready = false;
			}
			LOG4CXX_DEBUG(logger, "got frame in main th. fr-index: " << rc_frames.front().frame_index << ", fr-size: "
																	 << rc_frames.front().frame_size);
			if(rc_frames.size() > cfg.num_frame_to_keep)
				rc_frames.pop_back();

			if(!fr_proc)
				fr_proc = get_fr_processor(cfg, cam_cfg, rc_frames.front().cam_pro_frame->header.width, rc_frames.front().cam_pro_frame->header.height, cam_index);

			fr_proc->proc_next_frame({&rc_frames.front().mat, &rc_frames.front().cam_pro_frame->header, rc_frames.front().ipc_start_time});

			cv::pollKey();
		}
		LOG4CXX_INFO(logger, "stop receiving frames");
		return 0;
	}
}

