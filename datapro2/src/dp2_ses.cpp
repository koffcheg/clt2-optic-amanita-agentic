#include "dp2_ses.h"
#include "log4cxx/logger.h"
#include <boost/asio/ts/buffer.hpp>

using namespace dp2;
static auto logger = log4cxx::Logger::getLogger("dp2-ses");
static int last_ses_cnt = 0;

session::session(boost::asio::io_context &io_context, boost::asio::ip::tcp::socket socket, std::unique_ptr<rpc_sink> rpc_client, bool (*stop_work_func)()) :
		ses_index_{++last_ses_cnt},
		socket_(std::move(socket)),
		sentinel_timer_(io_context, boost::asio::chrono::seconds(2)),
		rpc_client_{std::move(rpc_client)},
		stop_work_func_(stop_work_func){
	LOG4CXX_DEBUG(logger, "cr. new ses: " << ses_index_);
}

void session::start() {
	auto self(shared_from_this());
	sentinel_timer_.async_wait([this, self](const boost::system::error_code &e) {
		ck_conn_alive(e);
	});
	do_read();
}

void session::do_read() {
	LOG4CXX_DEBUG(logger, "ses: " << ses_index_);
	auto self(shared_from_this());
	socket_.async_read_some(
			boost::asio::buffer(data_, max_length), [this, self](boost::system::error_code ec, std::size_t length) {
				on_read(ec, length);
			});
}

void session::on_read(boost::system::error_code ec, std::size_t length) {
	if (ec) {
		bool need_cancel_time{true};
		switch (ec.value()) {
			case boost::asio::error::eof:
				LOG4CXX_INFO(logger, "end of file, ses: " << ses_index_);
				break;
			case boost::system::errc::operation_canceled:
				LOG4CXX_INFO(logger, "canceled, ses: " << ses_index_);
				need_cancel_time = false;	//if someone cancel socket, then he will cancel timer too
				break;
			default:
				LOG4CXX_ERROR(logger, "read error, ses: " << ses_index_ << " - " << ec.value() << " " << ec.message());
		}
		if(need_cancel_time){
			LOG4CXX_DEBUG(logger, "cancel timer");
			sentinel_timer_.cancel();
		}
		return;
	}
	LOG4CXX_DEBUG(logger, "ses: " << ses_index_ << ", read bytes:" << length);
	bool need_br_conn{false};
	rpc_client_->on_next_raw_read(reinterpret_cast<const uint8_t *>(data_), length, need_br_conn);
	if(need_br_conn){
		LOG4CXX_DEBUG(logger, "ses: " << ses_index_ << ", break conn. detected");
		sentinel_timer_.cancel();
		socket_.cancel();
		return;
	}
	do_read();
//	do_write(length);
}

void session::do_write(std::size_t length) {
	LOG4CXX_DEBUG(logger, "ses: " << ses_index_);
	auto self(shared_from_this());
	boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
							 [this, self](boost::system::error_code ec, std::size_t length) {
								 if (ec) {
									 LOG4CXX_ERROR(logger, "err. on write, ses: " << ses_index_);
								 } else {
									 LOG4CXX_DEBUG(logger, "ses: " << ses_index_ << ", wrote bytes:" << length);
									 do_read();
								 }
							 });
}

void session::ck_conn_alive(const boost::system::error_code &ec) {
	if (ec) {
		switch (ec.value()) {
			case boost::system::errc::operation_canceled:
				LOG4CXX_INFO(logger, "timer canceled, ses: " << ses_index_);
				break;
			default:
				LOG4CXX_ERROR(logger, "timer err., ses: " << ses_index_ << " - " << ec.value() << " " << ec.message());
		}
		return;
	}

	if (stop_work_func_()) {
		LOG4CXX_INFO(logger, "stop detected, ses " << ses_index_);
		socket_.cancel();
		return;
	}

	LOG4CXX_DEBUG(logger, "on timer, ses: " << ses_index_);
	sentinel_timer_.expires_from_now(boost::asio::chrono::milliseconds(450));
	auto self(shared_from_this());
	sentinel_timer_.async_wait([this, self](const boost::system::error_code &e) {
		ck_conn_alive(e);
	});
}

