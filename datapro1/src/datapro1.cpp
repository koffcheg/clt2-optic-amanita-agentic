//
// Created by user on 01.06.24.
//
#include "datapro1.h"
#include <iostream>
#include <fstream>
#include <condition_variable>
#include <thread>
#include <log4cxx/logger.h>
#include "dataproSaveToFile.h"
#include <filesystem>
#include <jansson.h>
#include "datetime.h"
#include "dataproFilter.h"
#include "m_json_unique_ptr.h"
#include "dp1_calc_limit.h"

using namespace ns_datapro1;
using std::vector;
using std::thread;
using std::string;

static log4cxx::LoggerPtr logger;

extern bool is_program_stop();

void init_dp1_multi_th_proc_logger(int cam_index) {
	logger = log4cxx::Logger::getLogger("dp1-" + std::to_string(cam_index) + ".fr-proc.th-impl");
}

void initializingParamDatapro1(const ns_datapro1::prg_config::cfg_subtractor &subtractor,
							   const unsigned int tiles_factor, const int &def_border, const int &frame_width,
							   const int &frame_height,
							   const ns_datapro1::prg_config::cfg_one_filter &corr,
							   const ns_datapro1::prg_config::cfg_one_filter &matched,
							   const ns_datapro1::prg_config::cfg_one_filter &blur,
							   TDataproConfig &data_param, TDataproVar &var) {

	calcNumberFrag(tiles_factor, frame_width, frame_height,
				   data_param.numFragX, data_param.numFragY);
	calcBorderSize(corr, matched, blur, def_border,
				   data_param.border_x, data_param.border_y);
	calcSizePart(frame_height, frame_width,
				 data_param.numFragY, data_param.numFragX,
				 data_param.sizePartY, data_param.sizePartX,
				 data_param.sizePartYend, data_param.sizePartXend);

	data_param.update_bg_model = subtractor.detectShadows;

	int index;
//    cv::namedWindow("BG", cv::WINDOW_NORMAL);
	var.vec_bgsubtractor.resize(tiles_factor);
	var.vec_frag.resize(tiles_factor);
	var.vec_bgmask.resize(tiles_factor);

	for (int i = 0; i < data_param.numFragY; ++i) {
		for (int j = 0; j < data_param.numFragX; ++j) {
			index = j + data_param.numFragX * i;
			if (subtractor.subtractorKNN)
				var.vec_bgsubtractor[index] = cv::createBackgroundSubtractorKNN(subtractor.historyKNN,
																				subtractor.historyKNN,
																				subtractor.detectShadows);//cv::bgsegm::createBackgroundSubtractorCNT();
			else
				var.vec_bgsubtractor[index] = cv::createBackgroundSubtractorMOG2(subtractor.historyMOG2,
																				 subtractor.varThresholdMOG2,
																				 subtractor.detectShadows);
		}
	}
}

void check_and_create_directories(const std::string& path){
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }
}

static void datapro1_calc_func(int i, int j, cv::Mat &tmp_frame, const TDataproConfig &data_param, TDataproVar &var,
							   const prg_config::cfg_segment &segment,
                               const prg_config::cfg_filters &filters,
                               const cv::Mat &KernelGauss,
							   std::vector<std::vector<TOptionsMeasurement>> &vec_meas,
                               const std::string &file_name,
                               const ns_datapro1::prg_config::cfg_test &test,
                               const TFolder& folder_name_dp1,
							   vector<vector<TDrawMeasurement>> &data_draw) {
	int index = j + data_param.numFragX * i;
//    cv::Mat tmp_mediana;
	int shift_x = data_param.sizePartX * j - data_param.border_x;
	int shift_y = data_param.sizePartY * i - data_param.border_y;
    cv::Mat tmp;
    std::vector<int> compression_params;
    std::string tmp_folder;
    cv::Rect roi;
    if (filters.corr.switched) {
        shift_x = data_param.sizePartX * j - data_param.border_x + filters.corr.width/2;
        shift_y = data_param.sizePartY * i - data_param.border_y + filters.corr.height/2;
    }
//    var.vec_data_draw[index].clear();
    if (test.debug){
        compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(9);
        tmp_folder = test.out_folder + folder_name_dp1.frame_input;
        check_and_create_directories(tmp_folder);
        cv::imwrite(tmp_folder + "/" + file_name + ".png", tmp_frame, compression_params);
    }
    var.vec_frag[index] = splitImage(tmp_frame, data_param.border_x, data_param.border_y, data_param.numFragX,
									 data_param.numFragY,
									 data_param.sizePartX, data_param.sizePartY,
									 data_param.sizePartXend, data_param.sizePartYend,
									 i, j);
	var.vec_frag[index].convertTo(var.vec_frag[index], CV_8UC1, 1.0 / 256, 0);

    if(filters.median.switched){
        inverseMedian2DFilter(var.vec_frag[index],
                              filters.median.window_1_size,
                              filters.median.window_2_size,
                              filters.median.rejection, var.vec_frag[index]);
        if (test.debug){
            tmp_folder = test.out_folder + folder_name_dp1.frame_diff;
            check_and_create_directories(tmp_folder);
            cv::imwrite(tmp_folder + "/" + file_name + ".png", var.vec_frag[index], compression_params);//(roi)
        }
    }

    if (filters.corr.switched) {
        filter2DCovariance(var.vec_frag[index], KernelGauss, var.vec_frag[index]);
        if (test.debug){
            cv::Rect roi(data_param.border_x,
                         data_param.border_y,
                         data_param.sizePartX,
                         data_param.sizePartY);
            tmp_folder = test.out_folder + folder_name_dp1.frame_coor;
            check_and_create_directories(tmp_folder);
 //            cv::imshow("BG", var.vec_frag[index]);
            cv::imwrite(tmp_folder + "/" + file_name + ".png", var.vec_frag[index](roi), compression_params);//(roi)
        }
    }
	var.vec_bgsubtractor[index]->apply(var.vec_frag[index], var.vec_bgmask[index], data_param.update_bg_model ? -1 : 0);
//    if (test.debug){
//        cv::Rect roi(data_param.border_x,
//                     data_param.border_y,
//                     data_param.sizePartX,
//                     data_param.sizePartY);
//        tmp_folder = test.out_folder + folder_name_dp1.frame_segment;
//        check_and_create_directories(tmp_folder);
//        cv::imwrite(tmp_folder + "/" + file_name + ".png", var.vec_bgmask[index](roi), compression_params);
////        var.vec_bgsubtractor[index]->getBackgroundImage	(tmp);
////        tmp_folder = test.out_folder + folder_name_dp1.frame_background;
////        check_and_create_directories(tmp_folder);
////        cv::imwrite(tmp_folder + "/" + file_name + ".png", tmp(roi), compression_params);
//////        cv::imshow("BG", var.vec_bgmask[index](roi));
//	}
	vector<vector<cv::Point>>  vec_contours = refineSegments(var.vec_bgmask[index], segment.element_str_w, segment.element_str_h,
										 segment.number_iter,
										 segment.element_str_type, segment.level, segment.level_max,
										 segment.threshold_type,
										 segment.contour_retrieval, segment.contour_approximation, shift_x, shift_y);
//    var.vec_data_draw[index] = calc_contour_param(vec_contours,segment.min_segment_size, segment.max_segment_size);
    calc_contour_param(segment.min_segment_size, segment.max_segment_size, segment.max_height_to_width_ratio,
                       vec_contours,
                       data_draw[index]);
//	vec_meas[index] = calcMeasurement(var.vec_frag[index], vec_contours,
//                                      segment.min_segment_size, segment.max_segment_size,var.vec_data_draw[index]);
    vec_meas[index] = calcMeasurement(var.vec_frag[index], vec_contours, data_draw[index]);
//    cv::imshow("BG", var.vec_bgmask[index]);
}


struct dp1_th_proc_par {
	struct tile_id {
		int i;
		int j;
	};
	//calculate control variables
	std::condition_variable &start_proc_cv;
	std::mutex &start_proc_mut;
	vector<std::atomic_bool> &start_proc_flag;
	std::deque<tile_id> &tiles_to_proc;
	volatile std::atomic_uint &num_thread_ready;

	cv::Mat *tmp_frame{};
	const TDataproConfig *data_param{};
	TDataproVar *var{};
	const prg_config::cfg_segment *segment{};
	const prg_config::cfg_filters *filters{};
	cv::Mat *KernelGauss{};
	std::vector<std::vector<TOptionsMeasurement>> *vec_meas{};
	vector<vector<TDrawMeasurement>> *data_draw{};
	const std::string *file_name{};
	const ns_datapro1::prg_config::cfg_test *test{};
	const TFolder *folder_name_dp1{};

	void dispatch_calc(const int th_index) {
		static std::mutex queue_mut;
		while (true) {
			tile_id proc_tile_id{};
			{
				std::lock_guard lk(queue_mut);
				if (tiles_to_proc.empty())
					break;
				proc_tile_id = tiles_to_proc.front();
				tiles_to_proc.pop_front();
			}
			LOG4CXX_DEBUG(logger, "thread " << th_index << " start to proc frame i: " << proc_tile_id.i << ", j: "
											<< proc_tile_id.j);
			datapro1_calc_func(proc_tile_id.i, proc_tile_id.j, *tmp_frame, *data_param, *var,
							   *segment,
							   *filters,
							   *KernelGauss,
							   *vec_meas,
							   *file_name,
							   *test,
							   *folder_name_dp1,
							   *data_draw);
			LOG4CXX_DEBUG(logger, "thread " << th_index << " proc. frame complete");
		}
		++num_thread_ready;
	}
};


static void dp1_proc_thread(const int th_index, dp1_th_proc_par *th_par) {
	LOG4CXX_DEBUG(logger, "new thread " << th_index);
	while (!is_program_stop()) {
		LOG4CXX_DEBUG(logger, "thread " << th_index << " wait for calc iter");
		{
			std::unique_lock lk(th_par->start_proc_mut);
			if (!th_par->start_proc_cv.wait_for(lk, std::chrono::milliseconds(300), [=] {
				return th_par->start_proc_flag[th_index].load() || is_program_stop();
			})) {
				LOG4CXX_INFO(logger, "thread " << th_index << " - time-out");
				continue;
			}
		}
		if (is_program_stop()) {
			LOG4CXX_INFO(logger, "thread " << th_index << " - stop detected");
			break;
		}
		th_par->start_proc_flag[th_index] = false;
		LOG4CXX_DEBUG(logger, "thread " << th_index << " next calc iter");
		th_par->dispatch_calc(th_index);
		LOG4CXX_DEBUG(logger, "thread " << th_index << " next calc iter is complete");
	}
	LOG4CXX_INFO(logger, "thread " << th_index << " - completed");
}

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
			  const i_calc_tile_limit* calc_tile_limit,
			  unsigned int dp1_num_thread) {

	unsigned int num_tiles = data_param.numFragY * data_param.numFragX;

	static std::condition_variable start_proc_cv;
	static std::mutex start_proc_mut;
	static vector<std::atomic_bool> start_proc_flag(dp1_num_thread);
	static std::deque<dp1_th_proc_par::tile_id> tiles_to_proc;
	static volatile std::atomic_uint num_thread_ready{0};

	static dp1_th_proc_par th_proc_par{start_proc_cv, start_proc_mut, start_proc_flag, tiles_to_proc, num_thread_ready};
	static bool aux_thread_created{false};
	if (!aux_thread_created) {
		th_proc_par.data_param = &data_param;
        th_proc_par.file_name = &file_name;
        th_proc_par.test = &test;
        th_proc_par.folder_name_dp1 = &folder_name_dp1;
		unsigned int cr_th_index{0};
		while(++cr_th_index < dp1_num_thread){
			thread aux_th(dp1_proc_thread, cr_th_index, &th_proc_par);
			aux_th.detach();
		}
	}
	aux_thread_created = true;

	std::vector<std::vector<TOptionsMeasurement>> vec_meas(num_tiles);
	vector<vector<TDrawMeasurement>> data_draw(num_tiles);

	if ((tmp_frame.channels() != 1)) {//!frame.color &
		cvtColor(tmp_frame, tmp_frame, cv::COLOR_BGR2GRAY);
	}

	th_proc_par.tmp_frame = &tmp_frame;
	th_proc_par.var = &var;
	th_proc_par.segment = &segment;
    th_proc_par.filters = &filters;
    th_proc_par.KernelGauss = &KernelGauss;
	th_proc_par.vec_meas = &vec_meas;
	th_proc_par.data_draw = &data_draw;

	for(int i = 0; i < data_param.numFragY; ++i)
		for(int j = 0; j < data_param.numFragX; ++j){
			int tile_index = j + data_param.numFragX * i;
			if(calc_tile_limit->need_proc_tile(tile_index)){
				tiles_to_proc.emplace_back(i, j);
				LOG4CXX_DEBUG(logger, "tile " << tile_index << " (" << i << "," << j <<") scheduled to proc ");
			}else{
				LOG4CXX_DEBUG(logger, "tile " << tile_index << " (" << i << "," << j <<") skipped");
			}
		}

	if (!(tmp_frame.rows && tmp_frame.cols)) {
		LOG4CXX_DEBUG(logger, "frame is wrong, cols: " << tmp_frame.cols << ", rows" << tmp_frame.rows);
	}

	num_thread_ready = 0;
	for (auto &el: start_proc_flag)
		el = true;
	start_proc_cv.notify_all();
	th_proc_par.dispatch_calc(0);

	uint64_t cnt{};
	while (num_thread_ready != dp1_num_thread) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (is_program_stop())
			return;
		++cnt;
	}
	LOG4CXX_DEBUG(logger, "frame proc complete, cnt = " << cnt);

	//get all draw-result together
	out_meas.clear();
	for (const auto &el: vec_meas)
		out_meas.insert(out_meas.end(), el.begin(), el.end());

	var.data_draw.clear();
	for (const auto &frame_fraw: data_draw)
		var.data_draw.insert(var.data_draw.end(), frame_fraw.begin(), frame_fraw.end());

    for (int i = 0; i < (int)out_meas.size(); i++) {
        out_meas[i].id_obj = i;
    }
}

void showing(cv::Mat tmp_frame, std::vector<TDrawMeasurement> &data_draw, const bool& display_marks,
             const bool &disp_rot_rec, cv::Mat& out_frame) {
    cv::Point2f vtx[4];
    int i_next;
    out_frame = tmp_frame.clone();
    auto cols = (float) out_frame.cols, rows = (float) out_frame.rows;
    //cv::cvtColor(tmp_frame, tmp_frame, cv::COLOR_GRAY2BGR);
    out_frame.convertTo(out_frame, CV_8UC3, 1.0 / 256, 0);
    //cv::cvtColor(tmp_frame, tmp_frame, cv::COLOR_BGR2GRAY);
    cv::cvtColor(out_frame, out_frame, cv::COLOR_GRAY2BGR);
    if (!data_draw.empty() & display_marks) {
        for (const auto &el: data_draw) {
            if (!disp_rot_rec)
                rectangle(out_frame, el.rect, cv::Scalar(0, 0, 255));
            else {
                el.box.points(vtx);
                for (int j = 0; j < 4; j++) {
                    i_next = (j + 1) % 4;
                    if ((vtx[j].x > 0) & (vtx[j].y > 0) & (vtx[i_next].x > 0) & (vtx[i_next].y > 0) &
                        (vtx[j].x < cols) & (vtx[j].y < rows) & (vtx[i_next].x < cols) & (vtx[i_next].y < rows))
                        line(out_frame, vtx[j], vtx[i_next], cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
                }
            }
        }
        data_draw.clear();
        data_draw.shrink_to_fit();
    }
}

void save_res(const std::vector<TOptionsMeasurement> &out_meas, const TDataFrame &data_frame,
			  const std::string &path, const TDataCam &data_cam, const double &interval, std::ofstream &file_bin,
              const TFolder& folder_name_dp1,
			  std::time_t &t_file) {
	std::time_t t_current = std::time(nullptr);
	double delta_t;

	if (!file_bin.good()) {
		creating_file(path, t_current, data_cam.cam_index, t_file, folder_name_dp1, file_bin);
		save_data_cam(file_bin, data_cam);
		save_data_frame(file_bin, data_frame);
		save_data_meas(file_bin, out_meas);
	} else {
		delta_t = (double)(t_current - t_file) / 3600.0;
		if (delta_t < interval) {
			save_data_frame(file_bin, data_frame);
			save_data_meas(file_bin, out_meas);
		} else {
			file_bin.close();
			creating_file(path, t_current, data_cam.cam_index, t_file, folder_name_dp1, file_bin);
			save_data_cam(file_bin, data_cam);
			save_data_frame(file_bin, data_frame);
			save_data_meas(file_bin, out_meas);
		}

	}
}


static void seva_res_json(const std::vector<TOptionsMeasurement> &out_meas, const TDataFrame &data_frame,
						  const TDataCam &data_cam,
                          const TDataCalibrationFrame& param_frame,
                          const string &out_path, const string &file_name){
	auto json_root = json_unique_ptr_create(json_object());
	{
		json_t *cam_json = json_object();
		json_object_set_new(cam_json, "cam_index", json_integer(data_cam.cam_index));
		json_object_set_new(cam_json, "Xcam", json_real(data_cam.Xcam));
		json_object_set_new(cam_json, "Ycam", json_real(data_cam.Ycam));
		json_object_set_new(cam_json, "Zcam", json_real(data_cam.Zcam));
		json_object_set_new(cam_json, "pixel_width", json_real(data_frame.pixelWidth));
		json_object_set_new(cam_json, "pixel_height", json_real(data_frame.pixelHeight));
		json_object_set_new(cam_json, "focal_length", json_real(data_frame.focalLength));
		json_object_set_new(json_root.get(), "Camera data", cam_json);
	}
	{
		json_t *turret_json = json_object();
		json_object_set_new(turret_json, "Az", json_real(data_frame.Az));
		json_object_set_new(turret_json, "El", json_real(data_frame.El));
		json_object_set_new(turret_json, "V_az", json_real(data_frame.V_az));
		json_object_set_new(turret_json, "V_el", json_real(data_frame.V_el));
		json_object_set_new(json_root.get(), "Turret data", turret_json);
	}
	{
		json_t *data_frame_json = json_object();
		json_object_set_new(data_frame_json, "index_frame", json_integer(data_frame.index_frame));
		json_object_set_new(data_frame_json, "exposureStart",
							json_string(timePointToString(data_frame.exposureStart, "%Z %Y-%m-%d %H:%M:%S.").c_str()));
		json_object_set_new(data_frame_json, "exposureLength", json_real(data_frame.exposureLength));
		json_object_set_new(data_frame_json, "width", json_integer(data_frame.width));
		json_object_set_new(data_frame_json, "height", json_integer(data_frame.height));
		json_object_set_new(json_root.get(), "Data frame", data_frame_json);
	}
	{
		json_t *measures_json = json_object();
		json_object_set_new(measures_json, "Size", json_integer(static_cast<json_int_t>(out_meas.size())));
		json_t *meas_array_json = json_array();
		for(const auto &i:out_meas){
			json_t *curr_meas_json = json_object();
			json_object_set_new(curr_meas_json, "id_obj", json_integer(i.id_obj));
			json_object_set_new(curr_meas_json, "num_pix_obj", json_integer(i.num_pix_obj));
			json_object_set_new(curr_meas_json, "x_weight", json_real(i.x_weight));
			json_object_set_new(curr_meas_json, "y_weight", json_real(i.y_weight));
            json_object_set_new(curr_meas_json, "x_rec", json_real(i.x_rec));
            json_object_set_new(curr_meas_json, "y_rec", json_real(i.y_rec));
			json_object_set_new(curr_meas_json, "rec_height", json_real(i.rec_height));
			json_object_set_new(curr_meas_json, "rec_width", json_real(i.rec_width));
			json_object_set_new(curr_meas_json, "obj_angel", json_real(i.obj_angel));
			json_object_set_new(curr_meas_json, "obj_angel_moment", json_real(i.obj_angel_moment));
			json_object_set_new(curr_meas_json, "obj_eccentricity", json_real(i.obj_eccentricity));
			json_object_set_new(curr_meas_json, "mean_brightness_obj", json_real(i.mean_brightness_obj));
			json_object_set_new(curr_meas_json, "std_brightness_obj", json_real(i.std_brightness_obj));
			json_array_append_new(meas_array_json, curr_meas_json);
		}
		json_object_set_new(measures_json, "Measurements", meas_array_json);
		json_object_set_new(json_root.get(), "Measurements frame", measures_json);
	}
    {
        json_t *frame_calib_json = json_object();
        json_t *r_matrix_array_json = json_array();
        for(int r=0; r<3; r++)
            for(int c=0; c<3; c++)
                json_array_append_new(r_matrix_array_json, json_real(param_frame.R_matrix.at<double>(r,c)));

        json_object_set_new(frame_calib_json, "R matrix", r_matrix_array_json);
        json_t *t_vector_array_json = json_array();
        for(auto i=0; i<3; i++){
            json_array_append_new(t_vector_array_json, json_real(param_frame.t_vec.at<double>(i)));
        }
        json_object_set_new(frame_calib_json, "t vector", t_vector_array_json);
        json_object_set_new(json_root.get(), "Frame calibration", frame_calib_json);
    }
	string res_file_path = out_path + "/" + file_name + ".json";
	json_dump_file(json_root.get(), res_file_path.c_str(), JSON_INDENT(4));
}

void save_res(const std::vector<TOptionsMeasurement> &out_meas, const TDataFrame &data_frame,
              const std::string &path, const TDataCam &data_cam, const bool& txt_file,
              const TDataCalibrationCamera& param_cam, const TDataCalibrationFrame& param_frame,
              const TFolder& folder_name_dp1, const std::string &file_name, const bool& binocular, std::ofstream &file_bin) {
//    std::time_t t_current = std::time(nullptr);
    std::string out_path = path + "/" + folder_name_dp1.data_bin;
    if (!std::filesystem::exists(out_path)) {
        std::filesystem::create_directories(out_path);
    }
    int check_binocular;
    file_bin.open(out_path + "/" + file_name + ".blob", std::ios::out  | std::ios::binary);//| std::ios::app
    save_data_cam(file_bin, data_cam);
    save_data_frame(file_bin, data_frame);
    save_data_meas(file_bin, out_meas);
    if (binocular){
        check_binocular = 1;
        file_bin.write(reinterpret_cast<const char *>(&check_binocular), sizeof(check_binocular));
        save_data_cam_calibration(file_bin, param_cam);
        save_data_frame_calibration(file_bin, param_frame);
    } else {
        check_binocular = 0;
        file_bin.write(reinterpret_cast<const char *>(&check_binocular), sizeof(check_binocular));
    }
    file_bin.close();
    if(txt_file){
        seva_res_json(out_meas, data_frame, data_cam, param_frame, out_path, file_name);
    }

}
