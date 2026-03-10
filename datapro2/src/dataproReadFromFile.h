#ifndef DATAPROREADFROMFILE_H
#define DATAPROREADFROMFILE_H

#include <vector>
#include "datapro2Types.h"

void read_data_cam(std::ifstream &file_bin, TDataCam &data_cam);

void read_data_frame(std::ifstream &file_bin, TDataFrame &data_frame);

void read_data_meas(std::ifstream &file_bin, std::vector<TOptionsMeasurement> &out_meas);

TDataRes read_res(const std::string &file_path, int& check_binocular, TDataCalibrationCamera& param_cam, TDataCalibrationFrame& param_frame);

#endif //DATAPROREADFROMFILE_H
