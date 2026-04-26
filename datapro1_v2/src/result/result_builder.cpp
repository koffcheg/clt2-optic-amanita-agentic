#include "dp1v2/result/result_builder.hpp"

#include "dp1v2/stages/calibration_contract.hpp"

namespace dp1v2 {

TDataRes build_empty_result(const FrameContext &context) {
    TDataRes result{};
    result.data_cam = context.data_cam;
    result.data_frame = context.data_frame;
    result.meas.clear();
    result.calib_frame = TDataCalibrationFrame{};
    result.calib_frame.R_matrix = cv::Mat::eye(kRotationMatrixRows, kRotationMatrixCols, CV_64F);
    result.calib_frame.t_vec = cv::Mat::zeros(kPoseVectorRows, kPoseVectorCols, CV_64F);
    result.calib_frame.r_vec = cv::Mat::zeros(kPoseVectorRows, kPoseVectorCols, CV_64F);
    result.calib_frame.descriptors_scene = cv::Mat::zeros(kDefaultDescriptorRows, kDefaultDescriptorCols, CV_64F);
    return result;
}

} // namespace dp1v2
