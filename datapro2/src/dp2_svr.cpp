#include "dp2_svr.h"
#include "dp2_ses.h"
#include "log4cxx/logger.h"

using boost::asio::ip::tcp;
using namespace dp2;
static auto logger = log4cxx::Logger::getLogger("dp2-svr");


server::server(boost::asio::io_context &io_context, uint16_t port, rpc_cl_faric_t *cl_fabric, bool (*stop_work_func)()) :
		context_(io_context),
		acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
		socket_(io_context),
		sentinel_timer_(io_context, boost::asio::chrono::seconds(2)),
		cl_fabric_(cl_fabric),
		stop_work_func_(stop_work_func){
	do_accept();
	LOG4CXX_INFO(logger, "start listening at " << port);

	sentinel_timer_.async_wait([this](const boost::system::error_code &e) {
		on_timer(e);
	});
}

void server::do_accept() {
	acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
		if (ec) {
			switch (ec.value()) {
				case boost::system::errc::operation_canceled:
					LOG4CXX_INFO(logger, "canceled");
					break;
				default:
					LOG4CXX_ERROR(logger, "error: " << ec.value() << " - " << ec.message());
			}
			return;
		} else {
			boost::asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(ec);
			if (!ec)
				LOG4CXX_INFO(logger, "incoming conn. from " << endpoint.address().to_string());
			std::make_shared<session>(context_, std::move(socket_), cl_fabric_(), stop_work_func_)->start();
		}
		do_accept();
	});
}

void server::on_timer(const boost::system::error_code &ec) {
	if (ec) {
		switch (ec.value()) {
			case boost::system::errc::operation_canceled:
				LOG4CXX_INFO(logger, "timer canceled");
				break;
			default:
				LOG4CXX_ERROR(logger, "error: " << ec.value() << " " << ec.message());
		}
		return;
	}

	if (stop_work_func_()) {
		LOG4CXX_INFO(logger, "stop detected");
		acceptor_.cancel();
		return;
	}

	LOG4CXX_DEBUG(logger, "on timer");
	sentinel_timer_.expires_from_now(boost::asio::chrono::milliseconds(450));
	sentinel_timer_.async_wait([this](const boost::system::error_code &e) {
		on_timer(e);
	});
}
