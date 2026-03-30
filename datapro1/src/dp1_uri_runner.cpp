#include <log4cxx/logger.h>
#include "dp1_uri_runner.h"
#include "dataproOtherSource.h"
#include "dp1_frame_proc.h"
#include "camera_frame.hpp"

static log4cxx::LoggerPtr logger;

namespace ns_datapro1 {

	struct rc_uri_raw_frame {
		//ipc_rc_data_store frame;
		size_t frame_size{};
		size_t frame_index{};
		cv::Mat mat;
		cam_pro::FrameHeader cam_pro_header;
	};

	static rc_uri_raw_frame just_rc_frames;

	void init_uri_runner_logger(int cam_index) {
		logger = log4cxx::Logger::getLogger("dp1-" + std::to_string(cam_index) + ".uri-runner");
	}

	int run_uri_src(const prg_config &cgf, const TDataCalibrationCamera &cam_cfg, int cam_index, bool (*need_stop)()) {
		//TODO - add code for read frames from some URI

		static int num_proc_frames{};
		std::deque<rc_uri_raw_frame> rc_frames;
		std::unique_ptr<frame_processor> fr_proc;

		cv::VideoCapture cap;
		cv::Mat frame;
		choice_source(cgf.source, cap);
		if (!cap.isOpened()) {
			LOG4CXX_ERROR(logger, "can not open the source");
			return -1;
		}

		while (!need_stop()) {
			cap >> frame;
			if (frame.empty()) {
				if (cgf.source.source == "videofile" || cgf.source.source == "imagefile") {
					LOG4CXX_INFO(logger, "input stream is finished");
					break;
				}
				LOG4CXX_ERROR(logger, "can not read data from the source");
				return -1;
			}
//            std::cout << frame.channels() << std::endl;
//           std::cout << frame.type() << std::endl;
//            std::cout << frame.depth() << std::endl;
            cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
			frame.convertTo(frame, CV_16UC1, 256, 0);
			frame.copyTo(just_rc_frames.mat);
			just_rc_frames.frame_index = ++num_proc_frames;


			rc_frames.push_front(std::move(just_rc_frames));
			LOG4CXX_DEBUG(logger, "got frame in main th. fr-index: " << rc_frames.front().frame_index << ", fr-size: "
																	 << rc_frames.front().frame_size);
			if (rc_frames.size() > cgf.num_frame_to_keep)
				rc_frames.pop_back();

			if (!fr_proc)
				fr_proc = get_fr_processor(cgf, cam_cfg, frame.cols, frame.rows, cam_index);

			fr_proc->proc_next_frame({&rc_frames.front().mat, nullptr, std::chrono::steady_clock::now()});
			//cv::imshow("Video",rc_frames.front().mat);
			cv::pollKey();
		}
        cap.release();
		return 0;
	}

}
