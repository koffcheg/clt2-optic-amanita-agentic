#include "dp1v2/result/result_builder.hpp"

#include "dp1v2/stages/calibration_contract.hpp"

namespace {

constexpr float kDefaultFallbackPixelSizeM = 5E-6F;
constexpr float kDefaultFallbackFocalLengthM = 0.5F;
constexpr float kDefaultFallbackExposureLengthSec = 0.0F;
constexpr float kDefaultFallbackTurretAngleDeg = 0.0F;
constexpr float kDefaultFallbackTurretAngularVelocityDegPerSec = 0.0F;

} // namespace

namespace dp1v2 {

TDataRes build_empty_result(const FrameContext &context) {
    TDataRes result{};
    result.data_cam.cam_index = context.camera_id;

    result.data_frame.index_frame = static_cast<int>(context.frame_id);
    result.data_frame.dp1_spent_time = 0;
    if (context.acquisition_time.valid) {
        result.data_frame.exposureStart = context.acquisition_time.system_time;
    } else {
        result.data_frame.exposureStart = DateTime{};
    }
    result.data_frame.width = context.geometry.width;
    result.data_frame.height = context.geometry.height;
    result.data_frame.exposureLength = kDefaultFallbackExposureLengthSec;
    result.data_frame.focalLength = kDefaultFallbackFocalLengthM;
    result.data_frame.pixelWidth = kDefaultFallbackPixelSizeM;
    result.data_frame.pixelHeight = kDefaultFallbackPixelSizeM;
    result.data_frame.El = kDefaultFallbackTurretAngleDeg;
    result.data_frame.Az = kDefaultFallbackTurretAngleDeg;
    result.data_frame.V_el = kDefaultFallbackTurretAngularVelocityDegPerSec;
    result.data_frame.V_az = kDefaultFallbackTurretAngularVelocityDegPerSec;
    result.data_frame.turretInfoValid = false;

    result.meas.clear();
    result.calib_frame = TDataCalibrationFrame{};
    result.calib_frame.R_matrix = cv::Mat::eye(kRotationMatrixRows, kRotationMatrixCols, CV_64F);
    result.calib_frame.t_vec = cv::Mat::zeros(kPoseVectorRows, kPoseVectorCols, CV_64F);
    result.calib_frame.r_vec = cv::Mat::zeros(kPoseVectorRows, kPoseVectorCols, CV_64F);
    result.calib_frame.descriptors_scene = cv::Mat::zeros(kDefaultDescriptorRows, kDefaultDescriptorCols, CV_64F);
    return result;
}

} // namespace dp1v2
