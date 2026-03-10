#include "dataproReadFromFile.h"

#include <fstream>


void read_data_cam(std::ifstream &file_bin, TDataCam &data_cam) {
    file_bin.read(reinterpret_cast<char *>(&data_cam), sizeof(data_cam));
}

void read_data_frame(std::ifstream &file_bin, TDataFrame &data_frame) {
    file_bin.read(reinterpret_cast<char *>(&data_frame), sizeof(data_frame));
}

void read_data_meas(std::ifstream &file_bin, std::vector<TOptionsMeasurement> &out_meas) {
    size_t size_meas;
    file_bin.read(reinterpret_cast<char *>(&size_meas), sizeof(size_meas));
    if (size_meas > 0) {
        out_meas.resize(size_meas);
        file_bin.read(reinterpret_cast<char *>(out_meas.data()), size_meas * sizeof(TOptionsMeasurement));
    }
}
cv::Mat read_Mat_from_bin(std::ifstream &file_bin){
    int cols, rows, elemSizeInBytes, elemType;
    file_bin.read(reinterpret_cast<char *>(&cols), sizeof(cols));
    file_bin.read(reinterpret_cast<char *>(&rows), sizeof(rows));
    file_bin.read(reinterpret_cast<char *>(&elemSizeInBytes), sizeof(elemSizeInBytes));
    file_bin.read(reinterpret_cast<char *>(&elemType), sizeof(elemType));
    cv::Mat tmp_mat(rows, cols, elemType);
    file_bin.read(reinterpret_cast<char*>(&tmp_mat.data),cols*rows*elemSizeInBytes);
    cv::Mat /*tmp_mat(rows, cols, elemType, array),*/ matrix;
    tmp_mat.copyTo(matrix);
    return matrix;
}

void read_data_cam_calibration(std::ifstream &file_bin, TDataCalibrationCamera& param_cam) {
    param_cam.cameraMatrix = read_Mat_from_bin(file_bin);
    param_cam.distCoeffs = read_Mat_from_bin(file_bin);
    file_bin.read(reinterpret_cast<char *>(&param_cam.avg_reprojection_error), sizeof(param_cam.avg_reprojection_error));
    file_bin.read(reinterpret_cast<char *>(&param_cam.square_size), sizeof(param_cam.square_size));
    file_bin.read(reinterpret_cast<char *>(&param_cam.board_width), sizeof(param_cam.board_width));
    file_bin.read(reinterpret_cast<char *>(&param_cam.board_height), sizeof(param_cam.board_height));
}

void read_data_frame_calibration(std::ifstream &file_bin, TDataCalibrationFrame& param_frame) {
    param_frame.R_matrix = read_Mat_from_bin(file_bin);
    param_frame.t_vec = read_Mat_from_bin(file_bin);
    param_frame.r_vec = read_Mat_from_bin(file_bin);
    param_frame.descriptors_scene = read_Mat_from_bin(file_bin);
    size_t size_vector;
    file_bin.read(reinterpret_cast<char *>(&size_vector), sizeof(size_vector));
    param_frame.chessboard_corners.resize(size_vector);
    file_bin.read(reinterpret_cast<char *>(&param_frame.chessboard_corners[0]), size_vector * sizeof(cv::Point2f));

    file_bin.read(reinterpret_cast<char *>(&size_vector), sizeof(size_vector));
    param_frame.keypoints_scene.resize(size_vector);
    file_bin.read(reinterpret_cast<char *>(&param_frame.keypoints_scene[0]), size_vector * sizeof(cv::KeyPoint));
}

TDataRes read_res(const std::string &file_path, int& check_binocular, TDataCalibrationCamera& param_cam, TDataCalibrationFrame& param_frame) {
    std::ifstream file_bin(file_path, std::ios::in | std::ios::binary);
    if (!file_bin) {
        throw std::runtime_error("Error opening file: " + file_path);
    }

    TDataRes result;

    read_data_cam(file_bin, result.data_cam);
    read_data_frame(file_bin, result.data_frame);
    read_data_meas(file_bin, result.meas);

    file_bin.read(reinterpret_cast<char *>(&check_binocular), sizeof(check_binocular));
    if (check_binocular){
        read_data_cam_calibration(file_bin, param_cam);
        read_data_frame_calibration(file_bin, param_frame);
    }

    file_bin.close();

    return result;
}