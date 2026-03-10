#ifndef CLT_OPTIC_DP2_SERVER_H
#define CLT_OPTIC_DP2_SERVER_H

#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>
#include "m_rpc_sink.h"

namespace dp2 {
	using rpc_cl_faric_t = std::unique_ptr<rpc_sink> ();

	class server {
	public:
		server(boost::asio::io_context &io_context, uint16_t port, rpc_cl_faric_t *cl_fabric, bool (*stop_work_func)());

	private:
		void do_accept();

		void on_timer(const boost::system::error_code &ec);


		boost::asio::io_context &context_;
		boost::asio::ip::tcp::acceptor acceptor_;
		boost::asio::ip::tcp::socket socket_;
		boost::asio::steady_timer sentinel_timer_;
		rpc_cl_faric_t *cl_fabric_;
		bool (*stop_work_func_)();
	};
}

#endif //CLT_OPTIC_DP2_SERVER_H
