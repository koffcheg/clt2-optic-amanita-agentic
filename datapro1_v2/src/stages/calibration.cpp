#include "dp1v2/stages/calibration.hpp"

#include <stdexcept>

#include <opencv2/core.hpp>

namespace dp1v2 {

CalibrationState load_calibration_state(const CalibrationConfig &config) {
    CalibrationState state{};
    if (config.camera_settings_file.empty()) {
        if (config.required) {
            throw std::logic_error("calibration camera settings file is required");
        }
        return state;
    }

    cv::FileStorage fs(config.camera_settings_file, cv::FileStorage::READ);
    if (!fs.isOpened()) {
        if (config.required) {
            throw std::logic_error("can't open camera calibration file: " + config.camera_settings_file);
        }
        state.status = "calibration_file_not_open";
        return state;
    }

    fs["camera_matrix"] >> state.camera.cameraMatrix;
    fs["distortion_coefficients"] >> state.camera.distCoeffs;
    fs["square_size"] >> state.camera.square_size;
    fs["board_width"] >> state.camera.board_width;
    fs["board_height"] >> state.camera.board_height;
    fs["avg_reprojection_error"] >> state.camera.avg_reprojection_error;

    if (state.camera.cameraMatrix.empty() || state.camera.distCoeffs.empty()) {
        if (config.required) {
            throw std::logic_error("camera calibration file is missing camera_matrix or distortion_coefficients");
        }
        state = CalibrationState{};
        state.status = "calibration_file_incomplete";
        return state;
    }

    state.loaded_from_file = true;
    state.status = "loaded_from_file";
    return state;
}

} // namespace dp1v2
