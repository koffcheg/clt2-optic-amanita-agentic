#include "dp2_tr_to_turret.h"
#include <boost/asio/ip/tcp.hpp>
#include <log4cxx/logger.h>
#include "m_rpc_d_former.h"
#include "dp2_cfg.h"
#include <boost/asio.hpp>
#include "dp2_prg_stop.h"
#include "dp2_rpc_cl.h"
#include "mem_store.h"
#include "rpc_msg_defines.h"
#include "dp2_rpc_data_mrsh.h"
#include "datapro2.h"


using std::vector;
using std::unique_ptr;
using std::make_unique;
using namespace std::chrono;
using boost::asio::ip::tcp;

static unique_ptr<tcp::socket> turret_socket;
static unique_ptr<boost::asio::steady_timer> turret_exch_timer;
static rpc_data_former data_former;
static time_point<steady_clock> last_try_to_conn_time;
static const dp2::dp2_cfg *prg_cfg;
static boost::asio::io_context *io_context;

static auto logger = log4cxx::Logger::getLogger("dp2-tr2turret");

static void turret_exch_on_timer(const boost::system::error_code &ec);
static void connect_to_turret(){
	LOG4CXX_INFO(logger, "try to connect to turret - " << prg_cfg->turret_exch.host << ":" << prg_cfg->turret_exch.port);
	try{
		turret_socket = make_unique<tcp::socket>(*io_context);
		tcp::resolver resolver(*io_context);
		boost::asio::connect(*turret_socket, resolver.resolve(prg_cfg->turret_exch.host, std::to_string(prg_cfg->turret_exch.port)));
		LOG4CXX_INFO(logger, "connected");
		data_former.reset();
	}catch(...){
		turret_socket.reset(nullptr);
		LOG4CXX_ERROR(logger, "Error on connecting to turret: " << prg_cfg->turret_exch.host << ":" << prg_cfg->turret_exch.port);
	}
	last_try_to_conn_time = steady_clock::now();
}

void init_tr_to_turret(boost::asio::io_context &a_io_context, const dp2::dp2_cfg &cfg)
{
	prg_cfg = &cfg;
	io_context = &a_io_context;
	if(!cfg.turret_exch.enabled){
		LOG4CXX_DEBUG(logger, "Exchange vs turret is not enabled");
		return;
	}
	connect_to_turret();

	turret_exch_timer = make_unique<boost::asio::steady_timer>(a_io_context, boost::asio::chrono::seconds(2));
	turret_exch_timer->async_wait(turret_exch_on_timer);
}

static void tr_msg_to_turret(CMemStore &ms){
	auto raw_msg = data_former.form_next_msg(ms.data(), ms.size());
	boost::system::error_code ec;
	boost::asio::write(*turret_socket, boost::asio::buffer(raw_msg.data(), raw_msg.size()), ec);
	if (ec){
		LOG4CXX_ERROR(logger, "error on write: " << ec.value() << " " << ec.message());
		turret_socket->close();
		turret_socket.reset();
		last_try_to_conn_time = steady_clock::now();
	}
}

static void tr_to_turret_all_tracks()
{
	if(!turret_socket) {
		LOG4CXX_INFO(logger, "conn. isn't established, skip transmit all tracks");
		return;
	}
	const std::vector<Trajectory> curr_tracks = get_presented_trajectories();
	CMemStore ms;
	ms.write_native(dp2_to_turret_rpc_msg_tracks);
	ms.write_native<int32_t>(prg_cfg->dp2_id);
	ms.write_native<uint32_t>(curr_tracks.size());
	for(const auto &tr : curr_tracks)
		serialize_dp2_res(ms, tr);

	tr_msg_to_turret(ms);
	LOG4CXX_DEBUG(logger, "tr. all tracks: " << curr_tracks.size());
}

static void check_reconn(){
	if (dp2::is_prg_stop())
		return;

	time_point<steady_clock> now_time = steady_clock::now();
	if(now_time - last_try_to_conn_time > seconds(prg_cfg->turret_exch.reconn_interval_s))
		connect_to_turret();
}

static void turret_exch_on_timer(const boost::system::error_code &ec) {
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

	if (dp2::is_prg_stop()) {
		LOG4CXX_INFO(logger, "stop detected");
		if(turret_socket)
			turret_socket->close();
		return;
	}

	LOG4CXX_DEBUG(logger, "on timer");
	if(!turret_socket)
		check_reconn();

	if(turret_socket)
		tr_to_turret_all_tracks();

	turret_exch_timer->expires_from_now(milliseconds(prg_cfg->turret_exch.tr_interval_ms));
	turret_exch_timer->async_wait(turret_exch_on_timer);
}

static void notify_turret_some_tracks(uint16_t msg_type, const vector<const Trajectory*>& tracks, unsigned int spent_proc_time){
	CMemStore ms;
	ms.write_native(msg_type);
	ms.write_native<int32_t>(prg_cfg->dp2_id);
	ms.write_native<uint32_t>(spent_proc_time);
	ms.write_native<uint32_t>(tracks.size());
	for(const auto &tr : tracks)
		serialize_dp2_res(ms, *tr);

	tr_msg_to_turret(ms);
}

static void notify_turret_dropped_tracks(const std::vector<unsigned long> &dropped_tracks){
	CMemStore ms;
	ms.write_native(dp2_to_turret_rpc_msg_dropped_tracks);
	ms.write_native<int32_t>(prg_cfg->dp2_id);
	ms.write_native<uint32_t>(dropped_tracks.size());
	for(auto el : dropped_tracks)
		ms.write_native<uint64_t>(el);

	tr_msg_to_turret(ms);
}

void tr_updates_to_turret(const StrobeMethod *algo, unsigned int spent_proc_time){
	if(!turret_socket)
		return;

	auto presented_tracks = algo->get_presented_trajectories();
	vector<const Trajectory*> new_tracks, updated_tracks;
	for(const auto &tr : presented_tracks){
		if(algo->is_track_new(tr.id))
			new_tracks.push_back(&tr);
		if(algo->is_track_updated(tr.id))
			updated_tracks.push_back(&tr);
	}
	if(!updated_tracks.empty()) {
		notify_turret_some_tracks(dp2_to_turret_rpc_msg_update_tracks, updated_tracks, spent_proc_time);
		LOG4CXX_DEBUG(logger, "update tracks notified: " << updated_tracks.size());
	}
	if(!new_tracks.empty()) {
		notify_turret_some_tracks(dp2_to_turret_rpc_msg_new_tracks, new_tracks, spent_proc_time);
		LOG4CXX_DEBUG(logger, "new tracks notified: " << new_tracks.size());
	}

	const auto &dropped_tracks = algo->get_dropped_track_ids();
	if(!dropped_tracks.empty()) {
		notify_turret_dropped_tracks(dropped_tracks);
		LOG4CXX_DEBUG(logger, "dropped tracks notified: " << dropped_tracks.size());
	}
}

