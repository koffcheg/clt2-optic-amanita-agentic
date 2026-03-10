#include "dp1m_receiver.h"
#include "dp1_config.h"
#include "dp1_ipc.h"
#include <log4cxx/logger.h>
#include <span>
#include <fstream>
#include <deque>
#include <condition_variable>

static log4cxx::LoggerPtr logger;

struct rc_ipc_raw_frame {
	ipc_rc_data_store frame;
	size_t frame_size{};
	size_t frame_index{};
};

static rc_ipc_raw_frame just_rc_frames;
static bool next_frame_ready{false};
static std::mutex just_rc_frames_mut;
static std::condition_variable cv;

void ns_dp1mock_rc::init_logger(int cam_index) {
	logger = log4cxx::Logger::getLogger("dp1mock-" + std::to_string(cam_index) + ".rc");
}

static void save_test_frame(size_t fr_index, const uint8_t *frame_data, size_t frame_size) {
	std::string fname = "test_rc_frame_" + std::to_string(fr_index) + ".bin";
	std::ofstream os(fname, std::ios::binary | std::ios::trunc);
	if (!os) {
		LOG4CXX_ERROR(logger, "can't open file to write test_frame: " << fname);
		return;
	}
	LOG4CXX_INFO(logger, "write data to test_frame");
	os.write(reinterpret_cast<const char *>(frame_data), static_cast<std::streamsize>(frame_size));
	LOG4CXX_INFO(logger, "write data completed");
}

//invoke by receiving thread (not main)
static void on_rc_next_frame(ipc_rc_data_store frame, std::size_t frame_size, std::size_t frame_ipc_index) {
	{
		std::lock_guard lk{just_rc_frames_mut};

		just_rc_frames.frame = std::move(frame);
		just_rc_frames.frame_size = frame_size;
		just_rc_frames.frame_index = frame_ipc_index;

		next_frame_ready = true;
	}
	cv.notify_one();
}

void ns_dp1mock_rc::start_receive(const char *cfg_fname, int camera_index, bool (*need_stop)()) {
	using namespace std::chrono_literals;
	LOG4CXX_INFO(logger, "enter receiver.");
	ns_datapro1::prg_config cfg(cfg_fname, camera_index);
	auto receiver = ns_datapro1::get_ipc_data_receiver(cfg, need_stop);

	receiver->receive_data(on_rc_next_frame);
	std::deque<rc_ipc_raw_frame> rc_frames;
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
			rc_frames.push_back(std::move(just_rc_frames));
			next_frame_ready = false;
		}
		LOG4CXX_DEBUG(logger, "got frame in main th. fr-index: " << rc_frames.back().frame_index << ", fr-size: "
																 << rc_frames.back().frame_size);
		save_test_frame(rc_frames.back().frame_index, reinterpret_cast<const uint8_t *>(rc_frames.back().frame.get()),
						rc_frames.back().frame_size);
	}

}

