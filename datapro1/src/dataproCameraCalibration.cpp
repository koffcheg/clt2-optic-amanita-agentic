//
// Created by user on 03.11.24.
//

#include "dataproCameraCalibration.h"
#include <log4cxx/logger.h>
#include "opencv2/xfeatures2d.hpp"

static auto logger = log4cxx::Logger::getLogger("dp1-calibration");

void read_camera_settings(const std::string& file_settings, TDataCalibrationCamera& cam_settings){
    cv::FileStorage fs( cv::samples::findFile( file_settings ), cv::FileStorage::READ);
    if (!fs.isOpened()){
        LOG4CXX_ERROR(logger, "Failed to open " << file_settings);
		throw std::logic_error(std::string("can't open camera calibration file: ") + file_settings);
    }
    fs["camera_matrix"] >> cam_settings.cameraMatrix;
    fs["distortion_coefficients"] >> cam_settings.distCoeffs;
    fs["square_size"] >> cam_settings.square_size;
    fs["board_width"] >> cam_settings.board_width;
    fs["board_height"] >> cam_settings.board_height;
    fs["avg_reprojection_error"] >> cam_settings.avg_reprojection_error;
}

void calcChessboardCorners(const cv::Size& boardSize, const float& squareSize, std::vector<cv::Point3f>& corners){
    corners.resize(0);
    for( int i = 0; i < boardSize.height; i++ )
        for( int j = 0; j < boardSize.width; j++ )
            corners.emplace_back(float(j*squareSize),float(i*squareSize), 0);
}
bool poseEstimationFromCoplanarPoints(const cv::Mat& img, const TDataCalibrationCamera& param_camera,
                                      TDataCalibrationFrame& param_frame){
    cv::Size patternSize(param_camera.board_width, param_camera.board_height);
    bool found = findChessboardCorners(img, patternSize, param_frame.chessboard_corners);
    if (!found){
        return found;
    }

    std::vector<cv::Point3f> objectPoints;
    calcChessboardCorners(patternSize, param_camera.square_size, objectPoints);
    std::vector<cv::Point2f> objectPointsPlanar;
    for (auto & objectPoint : objectPoints){
        objectPointsPlanar.push_back(cv::Point2f(objectPoint.x, objectPoint.y));
    }
    std::vector<cv::Point2f> imagePoints;
    cv::undistortPoints(param_frame.chessboard_corners, imagePoints, param_camera.cameraMatrix, param_camera.distCoeffs);
    cv::Mat H = cv::findHomography(objectPointsPlanar, imagePoints);

    LOG4CXX_DEBUG(logger, "H:\n" << H);

    // Normalization to ensure that ||c1|| = 1
    double norm = sqrt(H.at<double>(0,0)*H.at<double>(0,0) +
                       H.at<double>(1,0)*H.at<double>(1,0) +
                       H.at<double>(2,0)*H.at<double>(2,0));
    H /= norm;
    cv::Mat c1  = H.col(0);
    cv::Mat c2  = H.col(1);
    cv::Mat c3 = c1.cross(c2);
    param_frame.t_vec = H.col(2);
    param_frame.R_matrix.create(3, 3, CV_64F);
    for (int i = 0; i < 3; i++){
        param_frame.R_matrix.at<double>(i,0) = c1.at<double>(i,0);
        param_frame.R_matrix.at<double>(i,1) = c2.at<double>(i,0);
        param_frame.R_matrix.at<double>(i,2) = c3.at<double>(i,0);
    }

    cv::Mat_<double> W, U, Vt;
    cv::SVDecomp(param_frame.R_matrix, W, U, Vt);
    param_frame.R_matrix = U*Vt;
    double det = cv::determinant(param_frame.R_matrix);
    if (det < 0){
        Vt.at<double>(2,0) *= -1;
        Vt.at<double>(2,1) *= -1;
        Vt.at<double>(2,2) *= -1;
        param_frame.R_matrix = U*Vt;
    }
    cv::Rodrigues(param_frame.R_matrix, param_frame.r_vec);
    return found;
}

void fillCameraRt(const std::vector<double>& R_vec,
                  const std::vector<double>& t_vec,
                  cv::Mat& R_mat,
                  cv::Mat& t_mat){
    if (R_vec.size() != 9 || t_vec.size() != 3)
        throw std::invalid_argument("R_vec must contain 9 elements and t_vec 3 elements");

    /* --- заповнюємо 3×3 --- */
    R_mat.create(3, 3, CV_64F);
    for (int r = 0, idx = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c, ++idx)
            R_mat.at<double>(r, c) = R_vec[idx];

    /* --- заповнюємо 1×3 (рядок) --- */
    t_mat.create(1, 3, CV_64F);
    for (int c = 0; c < 3; ++c)
        t_mat.at<double>(0, c) = t_vec[c];
}