//
// Created by user on 12.06.24.
//

#include "dataproSaveToFile.h"
#include <filesystem>

std::string indx_to_string(const int& index_cam, const int &width){
    std::string res;
    std::stringstream convert;
    convert << std::setw(width) << std::setfill('0') << index_cam;
    res = convert.str();
    return res;
}

void time_to_string(const std::time_t& t_current, const int& index_cam,
                    std::string& name_file){
    std::stringstream trans_time;
    trans_time << std::put_time(std::gmtime(&t_current), "_D%y%m%d_T%H%M%S.blob");
    name_file = "C" + indx_to_string(index_cam, 3) + trans_time.str();
}

void time_to_string(const std::time_t& t_current, const int& index_cam, const int& index_frame,
                    std::string& name_file){
    std::stringstream trans_time;
    trans_time << std::put_time(std::gmtime(&t_current), "_D%y%m%d_T%H%M%S");
    name_file = "C" + indx_to_string(index_cam, 3) + //trans_time.str() +
            "_F" + indx_to_string(index_frame, 6);
}

void save_data_frame(std::ofstream& file_bin, const TDataFrame& data_frame){
    file_bin.write(reinterpret_cast<const char*>(&data_frame), sizeof(data_frame));// save data frame TODO
}

void save_Mat_to_bin(std::ofstream& file_bin, const cv::Mat& mat) {
    cv::Mat tmp;
    tmp = mat.clone();

    int elemSizeInBytes = (int)tmp.elemSize();
    int elemType        = (int)tmp.type();
    int dataSize        = (int)(tmp.cols * tmp.rows * tmp.elemSize());

//    FILE* FP = fopen(filename.c_str(), "wb");
    int array_size_image[4] = {tmp.cols, tmp.rows, elemSizeInBytes, elemType };
//    fwrite(/* buffer */ sizeImg, /* how many elements */ 4, /* size of each element */ sizeof(int), /* file */ FP);
//    file_bin.write(reinterpret_cast<const char*>(&sizeImg[0]), 4*sizeof(int));
    file_bin.write(reinterpret_cast<const char*>(array_size_image), 4*sizeof(int));
//    fwrite(mat.data, mat.cols * mat.rows, elemSizeInBytes, FP);
//    file_bin.write(reinterpret_cast<const char*>(&mat.data[0]), dataSize);
    file_bin.write(reinterpret_cast<const char*>(tmp.data), dataSize);
//    fclose(FP);
}

void save_data_frame_calibration(std::ofstream& file_bin, const TDataCalibrationFrame& param_frame){
    size_t size_vector;
    save_Mat_to_bin(file_bin, param_frame.R_matrix);
    save_Mat_to_bin(file_bin, param_frame.t_vec);
    save_Mat_to_bin(file_bin, param_frame.r_vec);
    save_Mat_to_bin(file_bin, param_frame.descriptors_scene);
    size_vector = param_frame.chessboard_corners.size();
    file_bin.write(reinterpret_cast<const char *>(&size_vector), sizeof(size_vector));// save number blob
    file_bin.write(reinterpret_cast<const char *>(&param_frame.chessboard_corners[0]),
                   size_vector * sizeof(cv::Point2f));

    size_vector = param_frame.keypoints_scene.size();
    file_bin.write(reinterpret_cast<const char *>(&size_vector), sizeof(size_vector));// save number blob
    file_bin.write(reinterpret_cast<const char *>(&param_frame.keypoints_scene[0]),
                   size_vector * sizeof(cv::KeyPoint));
}

void save_data_meas(std::ofstream& file_bin, const std::vector<TOptionsMeasurement>& out_meas){
    size_t size_meas;
    if(!out_meas.empty()) {
        size_meas = out_meas.size();
        file_bin.write(reinterpret_cast<const char *>(&size_meas), sizeof(size_meas));// save number blob
        file_bin.write(reinterpret_cast<const char *>(&out_meas[0]),
                       size_meas * sizeof(TOptionsMeasurement));// save blob
    }
    else{
        size_meas = 0;
        file_bin.write(reinterpret_cast<const char *>(&size_meas), sizeof(size_meas));
    }
}

void save_data_cam(std::ofstream& file_bin, const TDataCam& data_cam){
    file_bin.write(reinterpret_cast<const char*>(&data_cam), sizeof(data_cam));// save data cam
}

void save_data_cam_calibration(std::ofstream& file_bin, const TDataCalibrationCamera& param_cam){
    save_Mat_to_bin(file_bin, param_cam.cameraMatrix);
    save_Mat_to_bin(file_bin, param_cam.distCoeffs);
    file_bin.write(reinterpret_cast<const char *>(&param_cam.avg_reprojection_error),sizeof(param_cam.avg_reprojection_error));
    file_bin.write(reinterpret_cast<const char *>(&param_cam.square_size),sizeof(param_cam.square_size));
    file_bin.write(reinterpret_cast<const char *>(&param_cam.board_width),sizeof(param_cam.board_width));
    file_bin.write(reinterpret_cast<const char *>(&param_cam.board_height),sizeof(param_cam.board_height));
}

void creating_file(const std::string& path, const std::time_t& t_current, const int& index_cam, std::time_t& t_file,
                   const TFolder& folder_name_dp1,
                   std::ofstream& file_bin){
    std::string out_path = path + "/" + folder_name_dp1.data_bin;
    if (!std::filesystem::exists(out_path)) {
        std::filesystem::create_directories(out_path);
    }
    std::string name_file;
    t_file = t_current;
    time_to_string(t_file, index_cam, name_file);
    file_bin.open(out_path + "/" + name_file, std::ios::out | std::ios::app | std::ios::binary);
}

void creating_file_name(const std::time_t& t_current, const int& index_cam,
                   const int& index_frame, std::string& name_file){
    time_to_string(t_current, index_cam, index_frame, name_file);
//    file_bin.open(path + "/" + name_file, std::ios::out | std::ios::app | std::ios::binary);
}

//void creating_file(const std::string& path, const std::time_t& t_current, const int& index_cam,
//                   const int& index_frame, std::ofstream& file_bin){
//    std::string name_file;
//    if (!std::filesystem::exists(path)) {
//        std::filesystem::create_directory(path);
//    }
//    time_to_string(t_current, index_cam, index_frame, name_file);
////    file_bin.open(path + "/" + name_file, std::ios::out | std::ios::app | std::ios::binary);
//}

