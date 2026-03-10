#ifndef CLT_OPTIC_DP2_SESSION_H
#define CLT_OPTIC_DP2_SESSION_H

#include <boost/asio/ts/internet.hpp>
#include "m_rpc_sink.h"

namespace dp2 {
	class session : public std::enable_shared_from_this<session> {
	public:
		session(boost::asio::io_context &io_context, boost::asio::ip::tcp::socket socket, std::unique_ptr<rpc_sink> rpc_client, bool (*stop_work_func)());

		void start();

	private:
		void do_read();

		void on_read(boost::system::error_code ec, std::size_t length);

		void do_write(std::size_t length);

		void ck_conn_alive(const boost::system::error_code & /*e*/);

		int ses_index_;
		boost::asio::ip::tcp::socket socket_;
		boost::asio::steady_timer sentinel_timer_;
		std::unique_ptr<rpc_sink> rpc_client_;
		bool (*stop_work_func_)();
		enum {
			max_length = 16384
		};
		char data_[max_length];
	};
}
#endif //CLT_OPTIC_DP2_SESSION_H
