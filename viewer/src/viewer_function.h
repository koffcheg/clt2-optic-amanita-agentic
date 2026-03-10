//
// Created by user on 01.09.24.
//

#ifndef CLT_OPTIC_VIEWER_FUNCTION_H
#define CLT_OPTIC_VIEWER_FUNCTION_H
#include <opencv2/opencv.hpp>
#include "cvui.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include "dataproReadFromFile.h"
#include "coordinateTransformation.h"
#include "calculationTrajectory.h"
#include <algorithm>
using std::string;

namespace fs = std::filesystem;
struct Txy{
    double x, y;
    int id;
};
struct TDataPro2{
    int cam_index;
    double Xcam = 0, Ycam = 0, Zcam = 0;
    double focal_length_x = 0, focal_length_y = 0, optical_centers_x = 0, optical_centers_y = 0;
    int frame_index;
    std::vector<Trajectory> trajectories;
};
bool get_arg(const cv::CommandLineParser& parser, int arg_index, std::string &res, const char *err_msg);
bool get_arg(const cv::CommandLineParser& parser, int arg_index, int& res, const char *err_msg);
std::string compose_file_name(const std::string &path, const std::string &filename, const std::string &ext);
std::vector<std::string> creating_list_files_folder(const std::string& path, const std::string& ext);
void calc_frame_dp1(cv::Mat& im, const std::string& path_bin_dp1, cv::Mat& frame, int& check_binocular);
void calc_frame_dp2(cv::Mat& im, const std::string& path_bin_dp1,
                    const std::vector<std::string>& files_name,
                    const size_t& index_current_frame,
                    cv::Mat& frame);

#endif //CLT_OPTIC_VIEWER_FUNCTION_H
