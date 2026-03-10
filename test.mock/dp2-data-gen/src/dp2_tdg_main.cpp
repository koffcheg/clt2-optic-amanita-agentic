#include <cstdlib>
#include <iostream>
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include <boost/asio.hpp>
#include <random>
#include "m_rpc_d_former.h"

using boost::asio::ip::tcp;
using std::vector;

static auto logger = log4cxx::Logger::getLogger("dp2-t-data-gen");
const std::string log_cfg_filename{"dp2t_dg_log.xml"};

static void test_sink_n_former(rpc_data_former &data_former, tcp::socket &s, int data_len) {
	using namespace std::chrono_literals;
	static std::random_device rd;
	static auto mtgen = std::mt19937{rd()};
	static auto ud = std::uniform_int_distribution<>{0, 255};
	vector<uint8_t> gen_data;
	gen_data.reserve(data_len);
	for (int i = 0; i < data_len; ++i)
		gen_data.push_back(ud(mtgen));
	auto raw_msg = data_former.form_next_msg(gen_data.data(), gen_data.size());
	boost::asio::write(s, boost::asio::buffer(raw_msg.data(), raw_msg.size()));

	raw_msg = data_former.check_heart_beat(false);
	if(!raw_msg.empty())
		boost::asio::write(s, boost::asio::buffer(raw_msg.data(), raw_msg.size()));

	std::this_thread::sleep_for(5ms);

	LOG4CXX_INFO(logger, "sent next message, size: " << data_len);
}


int main(int argc, char *argv[]) {
	try {
		if (argc != 3) {
			std::cerr << "Usage: async_tcp_echo_server <host port>\n";
			return 1;
		}

		auto configuration_status = log4cxx::xml::DOMConfigurator::configureAndWatch(log_cfg_filename);
		if (configuration_status == log4cxx::spi::ConfigurationStatus::NotConfigured) {
			std::cerr << "FATAL: " << "unable to configure logging subsystem from cfg.file: " << log_cfg_filename
					  << std::endl;
			return -1;
		}
		LOG4CXX_INFO(logger, "Entering application.");

		boost::asio::io_context io_context;
		tcp::socket s(io_context);
		tcp::resolver resolver(io_context);
		boost::asio::connect(s, resolver.resolve(argv[1], argv[2]));

		rpc_data_former data_former;
		for (int iter = 0; iter < 5; ++iter)
			for (int len = 0; len < 2048; ++len)
				test_sink_n_former(data_former, s, len);

		LOG4CXX_INFO(logger, "application completed");
	}
	catch (std::exception &e) {
		std::cerr << "Exception: " << e.what() << "\n";
		LOG4CXX_ERROR(logger, "exception: " << e.what());
	}

	return 0;
}
