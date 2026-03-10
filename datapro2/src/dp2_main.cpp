#include <iostream>
#include "dp2_svr.h"
#include "boost/asio/signal_set.hpp"
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include <filesystem>
#include "dp2_cfg.h"
#include "dp2_rpc_cl.h"
#include "dp2_tr_to_turret.h"

namespace fs = std::filesystem;

using std::string;

static auto logger = log4cxx::Logger::getLogger("dp2");
const std::string log_cfg_filename_pattern{"dp2_log.xml"};
static bool stop_program{false};

static void sig_h_stop(const boost::system::error_code &error, int signal_number) {
	if (error) {
		if (error.value() == boost::system::errc::operation_canceled)
			LOG4CXX_INFO(logger, "signal handler unset (operation_canceled): " << signal_number);
		else
			LOG4CXX_ERROR(logger, "there is an error on signal, error: " << error.value() << " - " << error.message());
	}
	LOG4CXX_INFO(logger, "signal received: " << signal_number);
	stop_program = true;
}

namespace dp2 {
	bool is_prg_stop(){return stop_program;}
}

int show_usage(const char *arg0) {
	std::cerr << "usage:\n        " << fs::path(arg0).filename().string() << " <config_file> [dp2_log.xml] " << std::endl;
	return -1;
}

static string get_log_config_fname_from_prg_fname(const string &prg_path) {
	fs::path p{prg_path};
	p = fs::absolute(p);
	fs::path cfg_path = p.parent_path();
	cfg_path /= log_cfg_filename_pattern;
	string res_str = cfg_path.string();
	return res_str;
}

int main(int argc, char *argv[]) {
	try {
		if (argc < 2 || argc > 3) {
			show_usage(argv[0]);
			return 1;
		}

		string log_cfg_filename;
		if(argc > 2)
			log_cfg_filename = argv[2];
		else
			log_cfg_filename = get_log_config_fname_from_prg_fname(argv[0]);
		auto configuration_status = log4cxx::xml::DOMConfigurator::configureAndWatch(log_cfg_filename);
		if (configuration_status == log4cxx::spi::ConfigurationStatus::NotConfigured) {
			std::cerr << "FATAL: " << "unable to configure logging subsystem from cfg.file: " << log_cfg_filename
					  << std::endl;
			return -1;
		}
		LOG4CXX_INFO(logger, "Entering application.");

		dp2::dp2_cfg cfg{argv[1]};
		dp2_init_measure_proc_algo(cfg.strobe_mth, cfg.binocular);

		boost::asio::io_context io_context;

		dp2::server s(io_context, cfg.port, dp2_rpc_cl::cr_sink, dp2::is_prg_stop);

		boost::asio::signal_set stop_signals(io_context, SIGINT, SIGTERM, SIGQUIT);
		stop_signals.async_wait(sig_h_stop);

		init_tr_to_turret(io_context, cfg);

		io_context.run();

		LOG4CXX_INFO(logger, "application completed");
	}
	catch (std::exception &e) {
		std::cerr << "Exception: " << e.what() << "\n";
		LOG4CXX_ERROR(logger, "exception: " << e.what());
	}

	return 0;
}
