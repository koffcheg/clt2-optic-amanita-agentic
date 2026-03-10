//
// Created by user on 12.06.24.
//

#ifndef CLT_OPTIC_DATAPROSAVETOFILE_H
#define CLT_OPTIC_DATAPROSAVETOFILE_H
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include "datarpoTypes.h"

void save_data_frame(std::ofstream& file_bin, const TDataFrame& data_frame);

void save_data_frame_calibration(std::ofstream& file_bin, const TDataCalibrationFrame& param_frame);

void save_data_meas(std::ofstream& file_bin, const std::vector<TOptionsMeasurement>& out_meas);

void save_data_cam(std::ofstream& file_bin, const TDataCam& data_cam);

void save_data_cam_calibration(std::ofstream& file_bin, const TDataCalibrationCamera& param_cam);

void creating_file(const std::string& path, const std::time_t& t_current, const int& index_cam, std::time_t& t_file,
                   const TFolder& folder_name_dp1,
                   std::ofstream& file_bin);
void creating_file_name(const std::time_t& t_current, const int& index_cam,
                   const int& index_frame, std::string& name_file);


#endif //CLT_OPTIC_DATAPROSAVETOFILE_H
