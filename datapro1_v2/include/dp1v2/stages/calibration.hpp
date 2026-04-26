#pragma once

#include "dp1v2/config/config.hpp"
#include "datarpoTypes.h"

namespace dp1v2 {

struct CalibrationState {
    TDataCalibrationCamera camera;
    bool loaded_from_file = false;
    const char *status = "default_calibration";
};

CalibrationState load_calibration_state(const CalibrationConfig &config);

} // namespace dp1v2
