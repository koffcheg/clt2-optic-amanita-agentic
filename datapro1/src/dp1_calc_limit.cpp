//
// Created by u on 09.10.24.
//

#include "dp1_calc_limit.h"
#include <set>
#include <cassert>
#include <sstream>
#include <log4cxx/logger.h>
#include "dp1_config.h"
#include "dp1_snail_path.h"

using std::map;
using std::vector;

static log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("dp1.fr-proc-skipper");

namespace ns_datapro1 {

	vector<bool> calc_need_fr_proc_statuses(double real_proc_time_rate, const map<double, double> &rates,
											const vector<unsigned int> &frame_priorities) {
		assert(frame_priorities.size());
		vector<bool> res(frame_priorities.size(), false);
		double cumulate_sum_limit = 1.01;
		auto bound_it = rates.lower_bound(real_proc_time_rate);
		if (bound_it != rates.begin()) {
			--bound_it;
			cumulate_sum_limit = bound_it->second;
		}
		double delta_sum = 1.0 / static_cast<double>(frame_priorities.size());
		auto it = frame_priorities.begin();
		res[*it] = true;
		double cumulate_sum = delta_sum;
		unsigned int num_proc = 1;
		for (++it; it != frame_priorities.end(); ++it) {
			cumulate_sum += delta_sum;
			if (cumulate_sum < cumulate_sum_limit) {
				res[*it] = true;
				++num_proc;
				assert(*it < res.size());
			} else
				break;
		}
		LOG4CXX_DEBUG(logger, "proc. rate for k: " << real_proc_time_rate << " is " << cumulate_sum_limit
												   << ", num proc " << num_proc << " of " << frame_priorities.size());
		return res;
	}

	class calc_tile_limit_impl : public i_calc_tile_limit {
		bool enable;
		std::size_t num_tile_x_, num_tile_y_;
		double ref_frame_proc_time_ms_;
		vector<unsigned int> frame_priorities_;
		vector<bool> fr_proc_statuses;
		map<double, double> rates_;
		double frame_proc_accum_time_ = 0.0;
		constexpr static const double time_upd_k = 0.4;

		void update_tile_stuses() {
			double time_rate = frame_proc_accum_time_ / ref_frame_proc_time_ms_;
			fr_proc_statuses = calc_need_fr_proc_statuses(time_rate, rates_, frame_priorities_);
			std::stringstream ss;
			ss << "eff. time: " << frame_proc_accum_time_ << ", k: " << time_rate << ", frame statuses:";
			int fr_index = 0;
			for (auto el: fr_proc_statuses) {
				ss << " " << fr_index++ << ":" << el;
			}
			LOG4CXX_DEBUG(logger, ss.str());
		}

	public:
		explicit calc_tile_limit_impl(const calc_tile_lim_cfg_t &cfg, std::size_t num_tile_x, std::size_t num_tile_y) :
				enable{cfg.enable},
				num_tile_x_{num_tile_x},
				num_tile_y_{num_tile_y},
				ref_frame_proc_time_ms_{cfg.ref_frame_proc_time_ms},
				rates_{cfg.rates} {
			snail_path snail(num_tile_y_, num_tile_x_);
			auto path = snail.get_path();
			for (const auto &el: path)
				frame_priorities_.push_back(el.first * num_tile_x_ + el.second);
#ifndef NDEBUG
			std::set<size_t> check_set;
			for (auto el: frame_priorities_)
				check_set.insert(el);
			assert(check_set.size() == num_tile_x_ * num_tile_y_);
#endif
			std::stringstream ss;
			ss << "frame priority:";
			for (auto el: frame_priorities_)
				ss << " " << el;
			LOG4CXX_DEBUG(logger, ss.str());
			update_tile_stuses();
		}

		[[nodiscard]] bool need_proc_tile(size_t tileIndex) const override {
			bool res = true;
			if (enable) {
				if (tileIndex < fr_proc_statuses.size())
					res = fr_proc_statuses[tileIndex];
			}
			return res;
		}

		void set_last_frame_proc_time_mc(unsigned int frame_proc_time) override {
			frame_proc_accum_time_ = time_upd_k * frame_proc_time + (1.0 - time_upd_k) * frame_proc_accum_time_;
			update_tile_stuses();
		}
	};

	std::unique_ptr<i_calc_tile_limit>
	get_tile_calc_limiter(const calc_tile_lim_cfg_t &cfg, std::size_t num_tile_x, std::size_t num_tile_y) {
		return std::make_unique<calc_tile_limit_impl>(cfg, num_tile_x, num_tile_y);
	}

}    //namespace
