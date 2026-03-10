//
// Created by user on 01.06.24.
//

#ifndef CLT_OPTIC_DATAPRO1_H
#define CLT_OPTIC_DATAPRO1_H
#include "datarpoTypes.h"
#include "dataproFilter.h"
#include "datarpoFragmentation.h"
#include "datarpoSegmentation.h"
#include "dp1_config.h"
#include <ctime>
#include <iomanip>
#include <iostream>

namespace ns_datapro1 {class i_calc_tile_limit;}

void init_dp1_multi_th_proc_logger(int cam_index);

//void getConfig(const ns_datapro1::prg_config& cfg, const int& frame_width, const int& frame_height, TParametersConfig& config);
void initializingParamDatapro1(const ns_datapro1::prg_config::cfg_subtractor&  subtractor,
							   const unsigned int tiles_factor, const int&  def_border, const int &frame_width, const int &frame_height,
							   const ns_datapro1::prg_config::cfg_one_filter& corr,
							   const ns_datapro1::prg_config::cfg_one_filter& matched,
							   const ns_datapro1::prg_config::cfg_one_filter& blur,
							   TDataproConfig &data_param, TDataproVar& var);
void datapro1(cv::Mat &tmp_frame, TDataproVar &var,
			  const TDataproConfig &data_param,
//			  const ns_datapro1::prg_config::cfg_frame &frame,
			  const ns_datapro1::prg_config::cfg_segment &segment,
              const std::string &file_name,
              const ns_datapro1::prg_config::cfg_test &test,
              const TFolder& folder_name_dp1,
              const ns_datapro1::prg_config::cfg_filters &filters,
              cv::Mat &KernelGauss,//const cv::Mat &KernelStroke,
			  std::vector<TOptionsMeasurement> &out_meas,
			  const ns_datapro1::i_calc_tile_limit*,
			  unsigned int dp1_num_thread);
void showing(cv::Mat tmp_frame, std::vector<TDrawMeasurement> &data_draw, const bool& display_marks,
             const bool &disp_rot_rec, cv::Mat& frame_show);
void save_res(const std::vector<TOptionsMeasurement>& out_meas, const TDataFrame& data_frame,
              const std::string& path, const TDataCam& data_cam, const double& interval, std::ofstream& file_bin,
              const TFolder& folder_name_dp1,std::time_t& t_file);
void save_res(const std::vector<TOptionsMeasurement> &out_meas, const TDataFrame &data_frame,
              const std::string &path, const TDataCam &data_cam, const bool& txt_file,
              const TDataCalibrationCamera& param_cam, const TDataCalibrationFrame& param_frame,
              const TFolder& folder_name_dp1, const std::string &file_name, const bool& binocular, std::ofstream &file_bin);

#endif //CLT_OPTIC_DATAPRO1_H