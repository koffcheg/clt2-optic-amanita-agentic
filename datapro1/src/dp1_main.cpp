#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/xml/domconfigurator.h>
#include <iostream>
#include <filesystem>
#include <atomic>
#include <thread>
#include <opencv2/core/utility.hpp>
#include "dp1_config.h"
#include "dp1_ipc_runner.h"
#include "dp1_frame_proc.h"
#include "ut_signal.h"
#include "dp1_ipc.h"
#include "dp1_uri_runner.h"
#include "dp1_tr_res2dp2.h"
#include "datarpoTypes.h"
#include "dataproCameraCalibration.h"

namespace fs = std::filesystem;
using namespace ns_datapro1;
using namespace std::chrono_literals;

static const std::string def_log_cfg_filename{"dp1_log.xml"};
static const std::string def_prg_cfg_filename{"config_datapro1.json"};
static auto logger = log4cxx::Logger::getLogger("dp1");

static std::atomic_bool program_stop{false};

bool is_program_stop(){return program_stop;}
void on_program_stop_request(){
	LOG4CXX_INFO(logger, "Stop detected");
	program_stop = true;
}

int show_usage(const char *arg0) {
	std::cerr << "usage:\n        " << fs::path(arg0).filename().string() << " <cam_index>  [config_datapro1.json] [dp1_log.xml]" << std::endl;
	return -1;
}

int get_camera_index(const char *arg) {
	int res_index = std::stoi(arg);
	return res_index;
}

bool parse_args(int argc, char *argv[], int &cam_index, std::string &prg_cfg_filename, std::string &log_cfg_filename){
	if(argc == 1)
		return false;
	cam_index = get_camera_index(argv[1]);

	if(argc > 2)
		prg_cfg_filename = argv[2];
	else
		prg_cfg_filename = def_prg_cfg_filename;

	if(argc > 3)
		log_cfg_filename = argv[3];
	else
		log_cfg_filename = def_log_cfg_filename;

	return true;
}

int main(int argc, char *argv[]) {
	int prg_ret{};
	try {
		std::string prg_cfg_filename;
		std::string log_cfg_filename;
		int cam_index;
		if(!parse_args(argc, argv, cam_index, prg_cfg_filename, log_cfg_filename))
			return show_usage(argv[0]);

		auto configuration_status = log4cxx::xml::DOMConfigurator::configureAndWatch(log_cfg_filename);
		if (configuration_status == log4cxx::spi::ConfigurationStatus::NotConfigured) {
			std::cerr << "FATAL: " << "unable to configure logging subsystem from cfg.file: " << log_cfg_filename
					  << std::endl;
			return -1;
		}

		logger = log4cxx::Logger::getLogger("dp1-" + std::to_string(cam_index));
		LOG4CXX_INFO(logger, "Entering application.");

		ns_datapro1::prg_config cfg{prg_cfg_filename.c_str(), cam_index};
        //ns_datapro1::prg_config::check_cfg(cfg);//TODO

		TDataCalibrationCamera cam_settings;
		read_camera_settings(cfg.binocular.file_camera_settings, cam_settings);

		ns_datapro1::init_ipc_logger(cam_index);
		ns_datapro1::init_ipc_runner_logger(cam_index);
		ns_datapro1::init_uri_runner_logger(cam_index);
		ns_datapro1::init_fr_proc_logger(cam_index);
		init_signal(std::string{"dp1-" + std::to_string(cam_index) + ".signal"}.c_str(), on_program_stop_request);
		init_connect_to_dp2(cfg.dp2_conn.host, cfg.dp2_conn.port, cfg.dp2_conn.reconn_interval_s, cam_index, cam_settings);

		if(cfg.multiproc.ocv_num_thread)
			cv::setNumThreads(cfg.multiproc.ocv_num_thread);

		switch(cfg.get_frame_src_type())
		{
			case prg_config::fr_src_ipc:
				prg_ret = run_ipc_src(cfg, cam_settings, cam_index, is_program_stop);
				break;
			case prg_config::fr_src_uri:
				prg_ret = run_uri_src(cfg, cam_settings, cam_index, is_program_stop);
				break;
			case prg_config::fr_src_invalid:
			default:
				LOG4CXX_ERROR(logger, "undefined frame source: " << cfg.source.source);
				return -1;
		}
		program_stop = true;
		LOG4CXX_INFO(logger, "runner has exited, wait some for ending working thread.");
		std::this_thread::sleep_for(500ms);	//wait for end all working threads
	} catch (const std::exception &e) {
		LOG4CXX_ERROR(logger, "std::exception - " << e.what());
		return -1;
	} catch (...) {
		LOG4CXX_ERROR(logger, "undefined exception");
		return -1;
	}
	LOG4CXX_INFO(logger, "normal closing application.");
	return prg_ret;
}
