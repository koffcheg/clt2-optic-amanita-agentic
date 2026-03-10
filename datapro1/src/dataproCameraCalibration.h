//
// Created by user on 03.11.24.
//
#include "datarpoTypes.h"
#ifndef CLT_OPTIC_DATAPROCAMERACALIBRATION_H
#define CLT_OPTIC_DATAPROCAMERACALIBRATION_H
void read_camera_settings(const std::string& file_settings, TDataCalibrationCamera& cam_settings);
bool poseEstimationFromCoplanarPoints(const cv::Mat& img, const TDataCalibrationCamera& param_camera,
                                      TDataCalibrationFrame& param_frame);
void calcChessboardCorners(const cv::Size& boardSize, const float& squareSize, std::vector<cv::Point3f>& corners);
void fillCameraRt(const std::vector<double>& R_vec,
                  const std::vector<double>& t_vec,
                  cv::Mat& R_mat,
                  cv::Mat& t_mat);
#endif //CLT_OPTIC_DATAPROCAMERACALIBRATION_H
