
#ifndef CLT_OPTIC_DP1_CALC_LIMIT_H
#define CLT_OPTIC_DP1_CALC_LIMIT_H

#include <memory>
#include <vector>
#include <map>

namespace ns_datapro1 {
	struct calc_tile_lim_cfg_t;

	class i_calc_tile_limit {
	public:
		virtual ~i_calc_tile_limit() = default;

		//request for neediness processing tile
		//may be used in multithreading
		//but not simultaneously with set_last_frame_proc_time_mc
		[[nodiscard]] virtual bool need_proc_tile(size_t tileIndex) const = 0;

		//update "next frame" processing time
		//don't allow multithreading any type (incl. simult. invoke with need_proc_tile)
		virtual void set_last_frame_proc_time_mc(unsigned int frame_proc_time) = 0;
	};

	std::vector<bool> calc_need_fr_proc_statuses(double real_proc_time_rate, const std::map<double, double> &rates,
												 const std::vector<unsigned int> &frame_priorities);

	std::unique_ptr<i_calc_tile_limit>
	get_tile_calc_limiter(const calc_tile_lim_cfg_t &cfg, std::size_t num_tile_x, std::size_t num_tile_y);
}

#endif //CLT_OPTIC_DP1_CALC_LIMIT_H
