#pragma once

#include "datarpoTypes.h"

namespace dp1v2 {

struct CalibrationState {
    TDataCalibrationCamera camera;
    bool loaded_from_file = false;
    const char *status = "default_calibration";
};

CalibrationState make_default_calibration_state();

} // namespace dp1v2
