#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/xml/domconfigurator.h>
#include <iostream>
#include <cstring>
#include <filesystem>
#include <atomic>
#include "dp1m_receiver.h"
#include "dp1_ipc.h"
#include "ut_signal.h"

namespace fs = std::filesystem;

static const std::string log_cfg_filename{"dp1mock_log.xml"};
static log4cxx::LoggerPtr logger;

static std::atomic_bool program_stop{false};

bool is_program_stop(){return program_stop;};
void on_program_stop_request(){
	LOG4CXX_INFO(logger, "Stop detected");
	program_stop = true;
}

int show_usage(const char *arg0) {
	std::cerr << "usage:\n        " << fs::path(arg0).filename().string() << " <cam_index>  <cfg_fname>" << std::endl;
	return -1;
}

int get_camera_index(const char *arg) {
	int res_index = std::stoi(arg);
	return res_index;
}

int main(int argc, char *argv[]) {
	auto configuration_status = log4cxx::xml::DOMConfigurator::configureAndWatch(log_cfg_filename);
	if (configuration_status == log4cxx::spi::ConfigurationStatus::NotConfigured) {
		std::cerr << "FATAL: " << "unable to configure logging subsystem from cfg.file: " << log_cfg_filename
				  << std::endl;
		return -1;
	}
	if (argc != 3 || strcmp(argv[1], "--help") == 0)
		return show_usage(argv[0]);
	try {
		auto cam_index = get_camera_index(argv[1]);
		logger = log4cxx::Logger::getLogger("dp1mock-" + std::to_string(cam_index));
		LOG4CXX_INFO(logger, "Entering application.");
		ns_dp1mock_rc::init_logger(cam_index);
		init_signal(std::string{"dp1-" + std::to_string(cam_index) + ".signal"}.c_str(), on_program_stop_request);

		ns_datapro1::init_ipc_logger(cam_index);

		ns_dp1mock_rc::start_receive(argv[2], cam_index, is_program_stop);

	} catch (const std::exception &e) {
		LOG4CXX_ERROR(logger, "std::exception - " << e.what());
		return -1;
	} catch (...) {
		LOG4CXX_ERROR(logger, "undefined exception");
		return -1;
	}
	LOG4CXX_INFO(logger, "normal closing application.");
	return 0;
}
