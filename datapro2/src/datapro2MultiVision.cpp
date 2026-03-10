//
// Created by user on 11.11.24.
//

#include "datapro2MultiVision.h"
#include <log4cxx/logger.h>
#include "calculationTrajectory.h"

static auto logger = log4cxx::Logger::getLogger("dp2-multiVision");
void computeC2MC1(const cv::Mat &R1, const cv::Mat &tvec1, const cv::Mat &R2, const cv::Mat &tvec2,
                  cv::Mat &R_1to2, cv::Mat &tvec_1to2){
    R_1to2 = R2 * R1.t();
    tvec_1to2 = R2 * (-R1.t()*tvec1) + tvec2;
}

cv::Mat computeHomography(const cv::Mat &R_1to2, const cv::Mat &tvec_1to2, const double& d_inv, const cv::Mat &normal){
    cv::Mat homography = R_1to2 + d_inv * tvec_1to2*normal.t();
    return homography;
}

cv::Mat computeHomography(const cv::Mat &R1, const cv::Mat &tvec1, const cv::Mat &R2, const cv::Mat &tvec2,
                      const double& d_inv, const cv::Mat &normal){
    cv::Mat homography = R2 * R1.t() + d_inv * (-R2 * R1.t() * tvec1 + tvec2) * normal.t();
    return homography;
}

void compute_camera_poses_two_images(const TDataCalibrationCamera& param_camera1, const TDataCalibrationCamera& param_camera2,
                                     const TDataCalibrationFrame& param_frame_cam1,const TDataCalibrationFrame& param_frame_cam2,
                                     cv::Mat& rvec1, cv::Mat& tvec1,
                                     cv::Mat& rvec2, cv::Mat& tvec2){
    std::vector<cv::Point3f> objectPoints1, objectPoints2;
    cv::Size patternSize_cam1(param_camera1.board_width, param_camera1.board_height),
            patternSize_cam2(param_camera2.board_width, param_camera2.board_height);
    calcChessboardCorners(patternSize_cam1, param_camera1.square_size, objectPoints1);
    calcChessboardCorners(patternSize_cam2, param_camera2.square_size, objectPoints2);

    cv::solvePnP(objectPoints1, param_frame_cam1.chessboard_corners,
                 param_camera1.cameraMatrix, param_camera1.distCoeffs, rvec1, tvec1);
    cv::solvePnP(objectPoints2, param_frame_cam2.chessboard_corners,
             param_camera2.cameraMatrix, param_camera2.distCoeffs, rvec2, tvec2);
}

std::vector<TOptionsMeasurement> convert_meas_cam1_to_cam2(const cv::Mat &homography,
                                                           const std::vector<TOptionsMeasurement> &meas_cam1){
    std::vector<TOptionsMeasurement> meas_cam1_to_cam2;
    for (const auto & i : meas_cam1){
        cv::Mat pt1 = (cv::Mat_<double>(3,1) << i.x_weight, i.y_weight, 1);
        cv::Mat pt2 = homography * pt1;
        pt2 /= pt2.at<double>(2);

//        meas_cam1_to_cam2[i]=meas_cam1[i];
        meas_cam1_to_cam2.push_back(i);
        meas_cam1_to_cam2.back().x_weight = pt2.at<double>(0);
        meas_cam1_to_cam2.back().y_weight = pt2.at<double>(1);
    }
    return meas_cam1_to_cam2;
}

std::vector<TDataRes> selection_meas_to_object_two_cameras(const cv::Mat &homography,
                                                           const TDataRes& dp1_res_cam1, const TDataRes& dp1_res_cam2,
                                                           const double &threshold_distance){

    std::vector<TDataRes> meas_match;
    std::vector<TOptionsMeasurement> match_meas_cam1, match_meas_cam2;
    std::vector<TOptionsMeasurement> meas_cam1_to_cam2;

    double distance, best_distance;
    int best_match_index1, best_match_index2;
    std::vector<bool> assigned_measurements(dp1_res_cam2.meas.size(), false);
    bool check;

    meas_cam1_to_cam2 = convert_meas_cam1_to_cam2(homography, dp1_res_cam1.meas);
    LOG4CXX_DEBUG(logger, "size c1 to c2 = " << meas_cam1_to_cam2.size());
    for (size_t i1 = 0; i1 < meas_cam1_to_cam2.size(); i1++){
        best_distance = std::numeric_limits<double>::max();
        best_match_index1 = -1;
        best_match_index2 = -1;
        check = false;
        for (size_t i2 = 0; i2 < dp1_res_cam2.meas.size(); i2++){
            if (!assigned_measurements[i2]){
                distance = sqrt(pow(meas_cam1_to_cam2[i1].x_weight - dp1_res_cam2.meas[i2].x_weight,2)+
                        pow(meas_cam1_to_cam2[i1].y_weight - dp1_res_cam2.meas[i2].y_weight,2));
//                LOG4CXX_DEBUG(logger, "distance = " << distance);
                if (distance < best_distance && distance <= threshold_distance){
                    best_distance = distance;
                    LOG4CXX_DEBUG(logger, "best distance = " << best_distance);
                    best_match_index1 = (int)i1;
                    best_match_index2 = (int)i2;
                    check = true;
                }
            }
        }
        if (check){
            match_meas_cam1.push_back(dp1_res_cam1.meas[best_match_index1]);
            match_meas_cam2.push_back(dp1_res_cam2.meas[best_match_index2]);
            assigned_measurements[best_match_index2] = true;
        }
    }
    TDataRes now_meas_cam1, now_meas_cam2;
    now_meas_cam1 = dp1_res_cam1;
    now_meas_cam2 = dp1_res_cam2;
    now_meas_cam1.meas = match_meas_cam1;
    now_meas_cam2.meas = match_meas_cam2;
    meas_match.push_back(now_meas_cam1);
    meas_match.push_back(now_meas_cam2);
    return meas_match;
}

std::vector<TDataRes> selection_meas_to_object_two_cameras(const TDataRes& dp1_res_cam1, const TDataRes& dp1_res_cam2,
                                                           const double &threshold_distance){
    std::vector<TDataRes> meas_match;
    // TODO check empty vector chessboard_corners!!!!!!!!!
    cv::Mat homography = findHomography(dp1_res_cam1.calib_frame.chessboard_corners, dp1_res_cam2.calib_frame.chessboard_corners);
//    cv::Mat homography = computeHomography(R_1to2, tvec_1to2, d_inv1, normal1);
    if ((!dp1_res_cam1.meas.empty())&&(!dp1_res_cam2.meas.empty())){
        LOG4CXX_DEBUG(logger, "Measurements number cam1, dp2: " << dp1_res_cam1.meas.size());
        LOG4CXX_DEBUG(logger, "Measurements number cam2, dp2: " << dp1_res_cam2.meas.size());
        meas_match = selection_meas_to_object_two_cameras(homography, dp1_res_cam1, dp1_res_cam2, threshold_distance);
        LOG4CXX_DEBUG(logger, "Measurements number match, dp2: cam1 - " << meas_match[0].meas.size() << ", cam2 - " << meas_match[1].meas.size());
    }
    return meas_match;
}

std::vector<int> get_number_cameras(const std::map<int, TDataCalibrationCamera>& all_cam_cfg){
    std::vector<int> num_cam;
    for (const auto & pItem : all_cam_cfg){
        num_cam.push_back(pItem.first);
    }
    return num_cam;
}

double calc_delta_time_between_frame(const std::deque<TDataRes>& vec_mes_cam){
    double delta_time_between_frame = 0;
    for (size_t i = 0; i < vec_mes_cam.size() - 1; i++){
        delta_time_between_frame += std::chrono::duration<double>(vec_mes_cam[i].data_frame.exposureStart - vec_mes_cam[i + 1].data_frame.exposureStart).count();
    }
    delta_time_between_frame /= double(vec_mes_cam.size() - 1);
    return delta_time_between_frame;
}

bool calc_shift_index_frame_(const std::deque<TDataRes>& vec_mes_cam1, const std::deque<TDataRes>& vec_mes_cam2,
                            size_t& index_cam1, size_t& index_cam2){
    bool range_check = false;
    size_t N1 = vec_mes_cam1.size(),
            N2 = vec_mes_cam2.size();
    std::deque<TDataRes> small_mes, big_mes;
    size_t index_min;
    double dt, min_dt = std::numeric_limits<double>::max();
    std::chrono::duration<double> DT{};

    if (N1 > N2){
        small_mes = vec_mes_cam2;
        big_mes = vec_mes_cam1;
    } else{
        small_mes = vec_mes_cam1;
        big_mes = vec_mes_cam2;
    }
    // variant = 1
    //  0 0 0
    //        . . . . . .
    //t=9 8 7 6 5 4 3 2 1
    // variant = 4 !ok
    //  0 0 0                 0 0
    //    . . . . . . .         . . . . . . .
    //t=8 7 6 5 4 3 2 1     t=8 7 6 5 4 3 2 1
    // variant = 2 !ok
    //    0 0 0           0 0 0 0 0 0       0 0 0 0 0           0 0 0 0 0
    //  . . . . . .       . . . . . .       . . . . . .       . . . . . .
    //t=6 5 4 3 2 1     t=6 5 4 3 2 1     t=6 5 4 3 2 1     t=6 5 4 3 2 1
    // variant = 5 !
    //          0 0 0               0 0
    //  . . . . . .       . . . . . .
    //t=7 6 5 4 3 2 1   t=7 6 5 4 3 2 1
    // variant = 3
    //              0 0 0
    //  . . . . . .
    //t=9 8 7 6 5 4 3 2 1
    if(small_mes.front().data_frame.exposureStart <= big_mes.front().data_frame.exposureStart &&
       small_mes.back().data_frame.exposureStart >= big_mes.back().data_frame.exposureStart){//        variant = 2;
        for (size_t i = 0; i < big_mes.size(); i++){
//            DT = big_mes[i].data_frame.exposureStart - small_mes.front().data_frame.exposureStart;
            DT = big_mes[i].data_frame.exposureStart - small_mes.back().data_frame.exposureStart;
            dt = std::fabs(DT.count());
            if (dt < min_dt){
                min_dt = dt;
                index_min = i;
            }
        }
        if (N1 > N2){
            index_cam1 = index_min;
            index_cam2 = small_mes.size()-1;
        } else{
            index_cam1 = small_mes.size()-1;
            index_cam2 = index_min;
        }
        range_check = true;
    } else if (small_mes.back().data_frame.exposureStart > big_mes.front().data_frame.exposureStart || //        variant = 1;
            small_mes.front().data_frame.exposureStart < big_mes.back().data_frame.exposureStart){//        variant = 3;
        range_check = false;
    } else if (small_mes.front().data_frame.exposureStart > big_mes.front().data_frame.exposureStart &&
               small_mes.back().data_frame.exposureStart <= big_mes.front().data_frame.exposureStart &&
               small_mes.back().data_frame.exposureStart > big_mes.back().data_frame.exposureStart){//        variant = 4;
        for (size_t i = 0; i < big_mes.size(); i++){
            DT = big_mes[i].data_frame.exposureStart - small_mes.back().data_frame.exposureStart;
            dt = std::fabs(DT.count());
            if (dt < min_dt){
                min_dt = dt;
                index_min = i;
            }
        }
        if (N1 > N2){
            index_cam1 = index_min;
            index_cam2 = small_mes.size()-1;
        } else{
            index_cam1 = small_mes.size()-1;
            index_cam2 = index_min;
        }
        range_check = true;
    } else if (small_mes.back().data_frame.exposureStart < big_mes.back().data_frame.exposureStart &&
               small_mes.front().data_frame.exposureStart >= big_mes.back().data_frame.exposureStart &&
               small_mes.front().data_frame.exposureStart < big_mes.front().data_frame.exposureStart){//        variant = 5;
        for (size_t i = 0; i < small_mes.size(); i++){
            DT = small_mes[i].data_frame.exposureStart - big_mes.back().data_frame.exposureStart;
            dt = std::fabs(DT.count());
            if (dt < min_dt){
                min_dt = dt;
                index_min = i;
            }
        }
        if (N1 > N2){
            index_cam1 = big_mes.size()-1;
            index_cam2 = index_min;
        } else{
            index_cam1 = index_min;
            index_cam2 = big_mes.size()-1;
        }
        range_check = true;
    }
    return range_check;
}

bool clear_shift_index_frame(std::deque<TDataRes>& vec_mes_cam1, std::deque<TDataRes>& vec_mes_cam2){
    bool range_check = false;
    size_t N1 = vec_mes_cam1.size(),
            N2 = vec_mes_cam2.size();
    std::deque<TDataRes> small_mes, big_mes;
    size_t index_min;
    double dt, min_dt = std::numeric_limits<double>::max();
    std::chrono::duration<double> DT{};

    if (N1 > N2){
        small_mes = vec_mes_cam2;
        big_mes = vec_mes_cam1;
    } else{
        small_mes = vec_mes_cam1;
        big_mes = vec_mes_cam2;
    }
    // variant = 1
    //  0 0 0
    //        . . . . . .
    //t=1 2 3 4 5 6 7 8 9
    // variant = 4 !ok
    //  0 0 0                 0 0
    //    . . . . . . .         . . . . . . .
    //t=1 2 3 4 5 6 7 8     t=1 2 3 4 5 6 7 8
    // variant = 2 !ok
    //    0 0 0           0 0 0 0 0 0       0 0 0 0 0           0 0 0 0 0
    //  . . . . . .       . . . . . .       . . . . . .       . . . . . .
    //t=1 2 3 4 5 6     t=1 2 3 4 5 6     t=1 2 3 4 5 6     t=1 2 3 4 5 6
    // variant = 5 !
    //          0 0 0               0 0
    //  . . . . . .       . . . . . .
    //t=1 2 3 4 5 6 7   t=1 2 3 4 5 6 7
    // variant = 3
    //              0 0 0
    //  . . . . . .
    //t=1 2 3 4 5 6 7 8 9
    if(small_mes.front().data_frame.exposureStart >= big_mes.front().data_frame.exposureStart &&
       small_mes.back().data_frame.exposureStart <= big_mes.back().data_frame.exposureStart){//        variant = 2;
        for (size_t i = 0; i < big_mes.size(); i++){
            DT = big_mes[i].data_frame.exposureStart - small_mes.front().data_frame.exposureStart;
            dt = std::fabs(DT.count());
            if (dt < min_dt){
                min_dt = dt;
                index_min = i;
            }
        }
        for (size_t j = 0; j < index_min; j++ ){
            big_mes.pop_front();
        }
        if (N1 > N2){
            vec_mes_cam1 = big_mes;
        } else{
            vec_mes_cam2 = big_mes;
        }
        range_check = true;
    } else if (small_mes.back().data_frame.exposureStart < big_mes.front().data_frame.exposureStart || //        variant = 1;
               small_mes.front().data_frame.exposureStart > big_mes.back().data_frame.exposureStart){//        variant = 3;
        LOG4CXX_DEBUG(logger, "Exception");
        range_check = false;
    } else if (small_mes.front().data_frame.exposureStart < big_mes.front().data_frame.exposureStart &&
               small_mes.back().data_frame.exposureStart >= big_mes.front().data_frame.exposureStart &&
               small_mes.back().data_frame.exposureStart < big_mes.back().data_frame.exposureStart){//        variant = 4;
        for (size_t i = 0; i < big_mes.size(); i++){
            DT = small_mes[i].data_frame.exposureStart - big_mes.front().data_frame.exposureStart;
            dt = std::fabs(DT.count());
            if (dt < min_dt){
                min_dt = dt;
                index_min = i;
            }
        }
        for (size_t j = 0; j < index_min; j++ ){
            small_mes.pop_front();
        }
        if (N1 > N2){
            vec_mes_cam2 = small_mes;
        } else{
            vec_mes_cam1 = small_mes;
        }
        range_check = true;
    } else if (small_mes.back().data_frame.exposureStart > big_mes.back().data_frame.exposureStart &&
               small_mes.front().data_frame.exposureStart <= big_mes.back().data_frame.exposureStart &&
               small_mes.front().data_frame.exposureStart > big_mes.front().data_frame.exposureStart){//        variant = 5;
        for (size_t i = 0; i < small_mes.size(); i++){
            DT = big_mes[i].data_frame.exposureStart - small_mes.front().data_frame.exposureStart;
            dt = std::fabs(DT.count());
            if (dt < min_dt){
                min_dt = dt;
                index_min = i;
            }
        }
        for (size_t j = 0; j < index_min; j++ ){
            big_mes.pop_front();
        }
        if (N1 > N2){
            vec_mes_cam1 = big_mes;
        } else{
            vec_mes_cam2 = big_mes;
        }
        range_check = true;
    }
    return range_check;
}

std::vector<cv::Point2f> convert_meas_to_vrector(const std::vector<TOptionsMeasurement>& meas){
    std::vector<cv::Point2f> points;
    for (const auto & mea : meas)
        points.emplace_back((float)mea.x_weight, (float)mea.y_weight);
    return points;
}

cv::Mat calc_projection_matrix(const cv::Mat& cameraMatrix, const cv::Mat& R_matrix, const cv::Mat& t_vec, cv::Mat& RT){
    RT  = cv::Mat(3, 4, CV_64F);
    R_matrix.copyTo(RT(cv::Rect(0,0,3,3)));
    t_vec.copyTo(RT(cv::Rect(3,0,1,3)));
    cv::Mat P = cameraMatrix * RT;
    return P;
}

void triangulatePoints(const TDataCalibrationCamera& param_camera1, const TDataCalibrationCamera& param_camera2,
                       const TDataRes& dp1_res_cam1, const TDataRes& dp1_res_cam2,
                       std::vector<double>& X, std::vector<double>& Y,std::vector<double>& Z){
    X.clear(); Y.clear(); Z.clear();
    if (!dp1_res_cam1.meas.empty() && !dp1_res_cam2.meas.empty()) {
        cv::Mat projMatr1, projMatr2, RT1, RT2, points4D;
        std::vector<cv::Point2f> vec_point1 = convert_meas_to_vrector(dp1_res_cam1.meas), undistort_vec_point1;
        std::vector<cv::Point2f> vec_point2 = convert_meas_to_vrector(dp1_res_cam2.meas), undistort_vec_point2;

        cv::undistortImagePoints(vec_point1, undistort_vec_point1, param_camera1.cameraMatrix,
                                 param_camera1.distCoeffs);
        cv::undistortImagePoints(vec_point2, undistort_vec_point2, param_camera2.cameraMatrix,
                                 param_camera2.distCoeffs);

        projMatr1 = calc_projection_matrix(param_camera1.cameraMatrix, dp1_res_cam1.calib_frame.R_matrix,
                                           dp1_res_cam1.calib_frame.t_vec, RT1);
        projMatr2 = calc_projection_matrix(param_camera2.cameraMatrix, dp1_res_cam2.calib_frame.R_matrix,
                                           dp1_res_cam2.calib_frame.t_vec, RT2);

        cv::triangulatePoints(projMatr1, projMatr2, undistort_vec_point1, undistort_vec_point2, points4D);
//        LOG4CXX_DEBUG(logger, "point1 = " << undistort_vec_point1.size() << ", point2 = " << undistort_vec_point2.size());
//        double fc_1 = .5*(param_camera1.cameraMatrix.at<double>(0,0) + param_camera1.cameraMatrix.at<double>(1,1)),
//                fc_2 = .5*(param_camera2.cameraMatrix.at<double>(0,0) + param_camera2.cameraMatrix.at<double>(1,1)),
//                c1x = param_camera1.cameraMatrix.at<double>(0,2),
//                c1y = param_camera1.cameraMatrix.at<double>(1,2),
//                c2x = param_camera2.cameraMatrix.at<double>(0,2),
//                c2y = param_camera2.cameraMatrix.at<double>(1,2),
//                zm = 100, std_measurement = 0.1, dispersion_xp, dispersion_yp, dispersion_zp; //TODO
//        std::vector<double> varX, varY, varZ;//TODO
        for (size_t i = 0; i < undistort_vec_point1.size(); i++) {
            X.push_back(points4D.at<float>(0, (int) i) / points4D.at<float>(3, (int) i));
            Y.push_back(points4D.at<float>(1, (int) i) / points4D.at<float>(3, (int) i));
            Z.push_back(points4D.at<float>(2, (int) i) / points4D.at<float>(3, (int) i));

//            calc_dispersion_measurement_3D_binocular(std_measurement, std_measurement,
//                                                     fc_1, fc_2, // mean focal length camera 1 & 2
//                                                     c1x, c1y, // center frame camera 1
//                                                     c2x, c2y, // center frame camera 2
//                                                     undistort_vec_point1[i].x, undistort_vec_point1[i].y, // coordinates of the object in the frame of the first camera
//                                                     undistort_vec_point2[i].x, undistort_vec_point2[i].y, // coordinates of the object in the frame of the second camera
//                                                     zm,                     // scale, distance plane of two frame
//                                                     dp1_res_cam1.calib_frame.R_matrix, dp1_res_cam1.calib_frame.t_vec,
//                                                     dp1_res_cam2.calib_frame.R_matrix, dp1_res_cam2.calib_frame.t_vec,
//                                                     dispersion_xp, dispersion_yp, dispersion_zp);
//            varX.push_back(dispersion_xp);
//            varY.push_back(dispersion_yp);
//            varZ.push_back(dispersion_zp);
        }
    }
}

TDataRes convert_corners_to_meas(const TDataRes& data_cam){
    TDataRes corners_cam;
    corners_cam = data_cam;
    corners_cam.meas.clear();
    corners_cam.meas.resize(data_cam.calib_frame.chessboard_corners.size());
    for (size_t i = 0; i < corners_cam.meas.size(); i++){
        corners_cam.meas[i].id_obj = (int)i;
        corners_cam.meas[i].x_weight = data_cam.calib_frame.chessboard_corners[i].x;
        corners_cam.meas[i].y_weight = data_cam.calib_frame.chessboard_corners[i].y;
        corners_cam.meas[i].num_pix_obj = 50;
    }
    return corners_cam;
}

void convert_corners_to_meas(const TDataRes& data_cam1, const TDataRes& data_cam2,
                             TDataRes& corners_cam1, TDataRes& corners_cam2){
    corners_cam1 = convert_corners_to_meas(data_cam1);
    corners_cam2 = convert_corners_to_meas(data_cam2);
}