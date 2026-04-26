#pragma once

#include "dp1v2/config/config.hpp"
#include "datarpoTypes.h"

namespace dp1v2 {

enum class ResultSinkStatus {
    Accepted,
    SendSkipped,
    ArtifactSkipped,
    Failed,
};

struct ResultSinkOutcome {
    ResultSinkStatus send_status = ResultSinkStatus::SendSkipped;
    ResultSinkStatus artifact_status = ResultSinkStatus::ArtifactSkipped;
    const char *reason = "";

    [[nodiscard]] bool ok() const {
        return send_status != ResultSinkStatus::Failed && artifact_status != ResultSinkStatus::Failed;
    }
};

const char *result_sink_status_to_cstr(ResultSinkStatus status);
void initialize_result_sink(const RuntimeConfig &config, int cam_index, const TDataCalibrationCamera &camera_calibration);
ResultSinkOutcome publish_result_to_sinks(const TDataRes &result);

} // namespace dp1v2
