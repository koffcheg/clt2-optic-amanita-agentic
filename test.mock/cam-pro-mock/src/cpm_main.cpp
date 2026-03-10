#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/xml/domconfigurator.h>
#include <iostream>
#include <cstring>
#include <filesystem>
#include "cp_config.h"
#include "cpm_generator.h"
#include <boost/json/src.hpp> // Include this only in one source file.

namespace fs = std::filesystem;

static const std::string log_cfg_filename{"cam_log.xml"};
static auto logger = log4cxx::Logger::getLogger("dp1");

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
	LOG4CXX_INFO(logger, "Entering application.");
	if (argc != 3 || strcmp(argv[1], "--help") == 0)
		return show_usage(argv[0]);
	try {
		cam_pro::Config cfg(argv[2], get_camera_index(argv[1]));
		cam_pro_test_generator cam_test_gen{cfg};
		cam_test_gen.run();
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
