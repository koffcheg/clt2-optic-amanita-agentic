#include <cstdlib>
#include <iostream>
#include "dp2_svr.h"
#include "boost/asio/signal_set.hpp"
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include "dp2_cfg.h"
#include "dp2m_rpc_cl.h"

static auto logger = log4cxx::Logger::getLogger("dp2-mock");
const std::string log_cfg_filename{"dp2mock_log.xml"};
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

int main(int argc, char *argv[]) {
	try {
		if (argc != 2) {
			std::cerr << "Usage: async_tcp_echo_server <config_file>\n";
			return 1;
		}

		auto configuration_status = log4cxx::xml::DOMConfigurator::configureAndWatch(log_cfg_filename);
		if (configuration_status == log4cxx::spi::ConfigurationStatus::NotConfigured) {
			std::cerr << "FATAL: " << "unable to configure logging subsystem from cfg.file: " << log_cfg_filename
					  << std::endl;
			return -1;
		}
		LOG4CXX_INFO(logger, "Entering application.");

		dp2::dp2_cfg cfg{argv[1]};

		boost::asio::io_context io_context;

		dp2::server s(io_context, cfg.port, cr_m_rpc_sink, dp2::is_prg_stop);

		boost::asio::signal_set stop_signals(io_context, SIGINT, SIGTERM, SIGQUIT);
		stop_signals.async_wait(sig_h_stop);

		io_context.run();

		LOG4CXX_INFO(logger, "application completed");
	}
	catch (std::exception &e) {
		std::cerr << "Exception: " << e.what() << "\n";
		LOG4CXX_ERROR(logger, "exception: " << e.what());
	}

	return 0;
}
