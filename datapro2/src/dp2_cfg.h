#ifndef CLT_OPTIC_DP2_CFG_H
#define CLT_OPTIC_DP2_CFG_H

#include <cstdint>
#include "dp2_srobe_mth_chg.h"

namespace dp2 {
	struct dp2_cfg {
		struct turret_exch_cfg
		{
			bool enabled;
			std::string host;
			uint16_t port;
			int tr_interval_ms;
			int reconn_interval_s;
		};

		explicit dp2_cfg(const char *cfg_fname);

		int dp2_id;
		uint16_t port{};
		dp2strobe_mth_cfg strobe_mth;
		turret_exch_cfg turret_exch;
//        data_camera_cfg data_camera;
        binocular_cfg binocular;
	};
}

#endif //CLT_OPTIC_DP2_CFG_H
