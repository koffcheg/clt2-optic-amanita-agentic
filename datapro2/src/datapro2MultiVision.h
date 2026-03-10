//
// Created by user on 11.11.24.
//

#ifndef CLT_OPTIC_DATAPRO2MULTIVISION_H
#define CLT_OPTIC_DATAPRO2MULTIVISION_H
#include "datapro1/src/datarpoTypes.h"
#include "datapro1/src/dataproCameraCalibration.h"
std::vector<TDataRes> selection_meas_to_object_two_cameras(const TDataRes& dp1_res_cam1, const TDataRes& dp1_res_cam2,
                                                           const double &threshold_distance);
std::vector<int> get_number_cameras(const std::map<int, TDataCalibrationCamera>& all_cam_cfg);
bool calc_shift_index_frame_(const std::deque<TDataRes>& vec_mes_cam1, const std::deque<TDataRes>& vec_mes_cam2,
                            size_t& index_cam1, size_t& index_cam2);
bool clear_shift_index_frame(std::deque<TDataRes>& vec_mes_cam1, std::deque<TDataRes>& vec_mes_cam2);
void triangulatePoints(const TDataCalibrationCamera& param_camera1, const TDataCalibrationCamera& param_camera2,
                       const TDataRes& dp1_res_cam1, const TDataRes& dp1_res_cam2,
                       std::vector<double>& X, std::vector<double>& Y,std::vector<double>& Z);
void convert_corners_to_meas(const TDataRes& data_cam1, const TDataRes& data_cam2,
                             TDataRes& corners_cam1, TDataRes& corners_cam2);
#endif //CLT_OPTIC_DATAPRO2MULTIVISION_H
