#include "tg_rc_from_dp2.h"
#include <atomic>
#include <thread>
#include <log4cxx/logger.h>
#include "datapro2/src/dp2_svr.h"
#include "mem_store.h"
#include "rpc_msg_defines.h"
#include "datapro2/src/dp2_rpc_data_mrsh.h"

using std::vector;

static std::atomic_bool stop_rc{};
static std::thread rc_thread;
static auto logger = log4cxx::Logger::getLogger("dp2-data-marsh");
static std::map<int, vector<Trajectory>> curr_tracks;	//dp2 --> dp2's tracks
std::mutex tracks_mut;


static void rc_thread_f(uint16_t listen_port);
static bool check_stop_work_func(){return stop_rc;}

static dp2_track_upd_func_notify_t f_new_tracks_notify = nullptr;
static dp2_track_upd_func_notify_t f_update_tracks_notify = nullptr;
static dp2_track_drop_func_notify_t f_drop_tracks_notify = nullptr;

void register_dp2_upd_tracks_callback(dp2_track_upd_func_notify_t notify_func){f_update_tracks_notify = notify_func;}

void register_dp2_new_tracks_callback(dp2_track_upd_func_notify_t notify_func){f_new_tracks_notify = notify_func;}

void register_dp2_drop_tracks_callback(dp2_track_drop_func_notify_t notify_func){f_drop_tracks_notify = notify_func;}

std::map<int, std::vector<Trajectory>> get_curr_tracks_from_dp2(){
	std::lock_guard lg(tracks_mut);
	std::map<int, std::vector<Trajectory>> res{curr_tracks};
	return res;
}

static void on_rd_dp2_all_tracks(CMemStore &ms){
	int32_t dp2_id;
	ms.read_native(dp2_id);

	uint32_t num_tracks;
	ms.read_native(num_tracks);
	vector<Trajectory> rc_tracks;
	for(uint32_t i = 0; i < num_tracks; ++i){
		Trajectory next_track;
		deserialize_dp2_res(ms, next_track);
		rc_tracks.push_back(next_track);
	}

	std::lock_guard lg(tracks_mut);
	curr_tracks[dp2_id] = std::move(rc_tracks);
}

static void on_rd_dp2_upd_or_new_tracks(CMemStore &ms, dp2_track_upd_func_notify_t notify_func){
	if(!notify_func)
		return;

	int32_t dp2_id;
	ms.read_native(dp2_id);

	uint32_t dp1_and_dp2_proc_time;
	ms.read_native(dp1_and_dp2_proc_time);

	uint32_t num_tracks;
	ms.read_native(num_tracks);
	vector<Trajectory> rc_tracks;
	for(uint32_t i = 0; i < num_tracks; ++i){
		Trajectory next_track;
		deserialize_dp2_res(ms, next_track);
		rc_tracks.push_back(next_track);
	}
	notify_func(dp2_id, dp1_and_dp2_proc_time, rc_tracks);
}

static void on_rd_dp2_dropped_tracks(CMemStore &ms){
	if(!f_drop_tracks_notify)
		return;

	int32_t dp2_id;
	ms.read_native(dp2_id);

	uint32_t num_tracks;
	ms.read_native(num_tracks);

	std::vector<unsigned long> dropped_tracks;
	for(uint32_t i = 0; i < num_tracks; ++i){
		uint64_t next_id;
		ms.read_native(next_id);
		dropped_tracks.push_back(next_id);
	}
	f_drop_tracks_notify(dp2_id, dropped_tracks);
}

class turret_rpc_cl : public rpc_sink {
	void on_rd_msg_complite(const uint8_t *data, std::size_t len) override {
		CMemStore ms(data, len);
		uint16_t msg_type;
		ms.read_native(msg_type);
		LOG4CXX_DEBUG(logger, "rc new msg, len: " << len << ", type: " << msg_type);
		switch (msg_type) {
			case dp2_to_turret_rpc_msg_tracks:		return on_rd_dp2_all_tracks(ms);
			case dp2_to_turret_rpc_msg_new_tracks:	return on_rd_dp2_upd_or_new_tracks(ms, f_new_tracks_notify);
			case dp2_to_turret_rpc_msg_update_tracks:	return on_rd_dp2_upd_or_new_tracks(ms, f_update_tracks_notify);
			case dp2_to_turret_rpc_msg_dropped_tracks:	return on_rd_dp2_dropped_tracks(ms);
			default:
				LOG4CXX_ERROR(logger, "undefined msg. type: " << msg_type);
		}
	}

	void on_heart_beat() override {
	}
public:
	static std::unique_ptr<rpc_sink> cr_sink(){
		return std::make_unique<turret_rpc_cl>();
	}
};

void init_rc_data_from_dp2(uint16_t listen_port){
	rc_thread = std::thread(rc_thread_f, listen_port);
}

void stop_rc_data_from_dp2(){
	if(!rc_thread.joinable()){
		LOG4CXX_DEBUG(logger, "rc-thread isn't joinable");
		return;
	}
	stop_rc = true;
	LOG4CXX_DEBUG(logger, "wait for join rc-thread");
	rc_thread.join();
	LOG4CXX_DEBUG(logger, "rc-thread joined");
}

static void rc_thread_f(uint16_t listen_port){
	LOG4CXX_DEBUG(logger, "start");

	boost::asio::io_context io_context;
	dp2::server s(io_context, listen_port, turret_rpc_cl::cr_sink, check_stop_work_func);
	LOG4CXX_DEBUG(logger, "server started at port " << listen_port);
	io_context.run();
	LOG4CXX_DEBUG(logger, "server stopeed");
}

