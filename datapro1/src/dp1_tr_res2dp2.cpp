#include "dp1_tr_res2dp2.h"
#include "mem_store.h"
#include <boost/asio.hpp>
#include <log4cxx/logger.h>
#include "dp1_rpc_data_mrsh.h"
#include "rpc_msg_defines.h"
#include "m_rpc_d_former.h"
#include "datarpoTypes.h"

using std::unique_ptr;
using boost::asio::ip::tcp;
using std::string;
using namespace std::chrono;

extern bool is_program_stop();

static unique_ptr<boost::asio::io_context> dp2_io_ctx;
static unique_ptr<tcp::socket> dp2socket;
static rpc_data_former data_former;
static string dp2_host;
static uint16_t dp2_port;
static seconds reconn_interval;
static time_point<steady_clock> last_try_to_conn_time;
static auto logger = log4cxx::Logger::getLogger("dp1-tr2dp2");
static const TDataCalibrationCamera *cam_settings;
static int cam_index;


static void tr_camera_settings();

static void connect_to_dp2(){
	LOG4CXX_INFO(logger, "try to connect to dp2 - " << dp2_host << ":" << dp2_port);
	try{
		dp2socket = std::make_unique<tcp::socket>(*dp2_io_ctx);
		tcp::resolver resolver(*dp2_io_ctx);
		boost::asio::connect(*dp2socket, resolver.resolve(dp2_host, std::to_string(dp2_port)));
		LOG4CXX_INFO(logger, "connected");
		dp2socket->set_option(boost::asio::ip::tcp::no_delay(true));
		LOG4CXX_INFO(logger, "no_delay option is set");
		data_former.reset();
		tr_camera_settings();
	}catch(...){
		dp2socket.reset(nullptr);
		LOG4CXX_ERROR(logger, "Error on connecting to dp2");
	}
	last_try_to_conn_time = steady_clock::now();
}

static void check_reconn(){
	if (is_program_stop())
		return;

	time_point<steady_clock> now_time = steady_clock::now();
	if(now_time - last_try_to_conn_time > reconn_interval)
		connect_to_dp2();
}

static bool tr_msg_to_dp2(CMemStore &ms){
	auto raw_msg = data_former.form_next_msg(ms.data(), ms.size());
	boost::system::error_code ec;
	boost::asio::write(*dp2socket, boost::asio::buffer(raw_msg.data(), raw_msg.size()), ec);
	if (ec){
		LOG4CXX_ERROR(logger, "error on write: " << ec.value() << " " << ec.message());
		dp2socket->close();
		dp2socket.reset();
		last_try_to_conn_time = steady_clock::now();
		return false;
	}else
		return true;
}

static void tr_camera_settings() {
	static CMemStore ms;
	ms.setPosition(0);

	ms.write_native(dp1_to_dp2_camera_calibration_data);
	ms.write_native(cam_index);
	serialize_camera_calibration_data(ms, *cam_settings);

	if (tr_msg_to_dp2(ms))
		LOG4CXX_DEBUG(logger, "sent camera settings");
	else
		LOG4CXX_ERROR(logger, "error on sending camera settings");
}

void ns_datapro1::send_res_to_dp2(const TDataRes& data) {
	if(!dp2socket){
		check_reconn();
		if(!dp2socket)
			return;
	}

	static CMemStore ms;
	ms.setPosition(0);

	ms.write_native(dp1_to_dp2_rpc_msg_new_measure);
	serialize_dp1_res(ms, data);

	if(tr_msg_to_dp2(ms))
		LOG4CXX_DEBUG(logger, "sent measurements: " << data.meas.size());
}

void ns_datapro1::init_connect_to_dp2(const std::string &a_dp2_host, uint16_t port, int reconn_interv_sec,  int a_cam_index, const TDataCalibrationCamera &a_cam_settings)
{
	dp2_host = a_dp2_host;
	dp2_port = port;
	reconn_interval = seconds (reconn_interv_sec);
	cam_index = a_cam_index;
	cam_settings = &a_cam_settings;
	dp2_io_ctx = std::make_unique<boost::asio::io_context>();
	connect_to_dp2();
}

void ns_datapro1::dp1_prc_check_tr_hb() {

}


