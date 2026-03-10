#include "cpm_generator.h"
#include "cp_config.h"
#include "cp_ipc_cam2dp1_if.h"
#include <log4cxx/logger.h>
#include <thread>
#include <span>
#include <fstream>

using namespace std::chrono_literals;

static auto logger = log4cxx::Logger::getLogger("cam-mock.gen");

static const size_t frame_size = 10 * 1024 * 1024 + 100;
static const size_t num_frame = 9;
static std::chrono::milliseconds delay_betw_frames{250ms};

cam_pro_test_generator::cam_pro_test_generator(const cam_pro::Config &prg_cfg) {
	data_forwarder = cam_pro::get_ipc_data_forwarder(frame_size, prg_cfg);
	LOG4CXX_INFO(logger, "created");
}

static void save_test_frame(size_t fr_index, std::span<uint8_t> frame) {
	std::string fname = "test_gen_frame_" + std::to_string(fr_index) + ".bin";
	std::ofstream os(fname, std::ios::binary | std::ios::trunc);
	if (!os) {
		LOG4CXX_ERROR(logger, "can't open file to write test_frame: " << fname);
		return;
	}
	LOG4CXX_INFO(logger, "write data to test_frame");
	os.write(reinterpret_cast<const char *>(frame.data()), static_cast<std::streamsize>(frame.size()));
	LOG4CXX_INFO(logger, "write data completed");
}

static void gen_frame(size_t fr_index, std::span<uint8_t> frame) {
	static uint8_t test_val = 41;
	LOG4CXX_INFO(logger, "start fill frame");
	for (auto &el: frame)
		el = ++test_val;
	LOG4CXX_INFO(logger, "end fill frame");
	save_test_frame(fr_index, frame);
}

void cam_pro_test_generator::run() {
	LOG4CXX_INFO(logger, "enter");
	for (size_t fr_index = 0u; fr_index < num_frame; ++fr_index) {
		auto *fr_ptr = reinterpret_cast<uint8_t *>(data_forwarder->get_prt_next_frame());
		LOG4CXX_INFO(logger, "start gen frame");
		gen_frame(fr_index, std::span{fr_ptr, frame_size});
		LOG4CXX_INFO(logger, "end gen frame");
		if(data_forwarder->tr_formed_frame()) {
			LOG4CXX_INFO(logger, "frame transferred via ipc");
		}
		std::this_thread::sleep_for(delay_betw_frames);
	}
}
