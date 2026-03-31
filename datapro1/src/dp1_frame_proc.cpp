#include "dp1_frame_proc.h"
#include <log4cxx/logger.h>
#include <fstream>
#include "camera_frame.hpp"
#include "datapro1.h"
#include "dp1_config.h"
#include "dataproMedianFilter.hpp"
#include "dataproMedianFilter2.hpp"
#include "dataproFilter.h"
#include "dp1_tr_res2dp2.h"
#include <filesystem>
#include "dataproSaveToFile.h"
#include "dp1_calc_limit.h"
#include "dataproCameraCalibration.h"
#include "datetime.h"
#include <stdexcept>
#include <array>
#include <algorithm>

static log4cxx::LoggerPtr logger;
using namespace std::chrono;
using namespace std::chrono_literals;
namespace ns_datapro1 {
    namespace {
        constexpr std::array<int, 3> kAllowedBinningFactors{1, 2, 4};
        constexpr const char *kBinningModeSum = "sum";

        bool is_valid_binning_factor(const int factor) {
            return std::find(kAllowedBinningFactors.begin(), kAllowedBinningFactors.end(), factor) != kAllowedBinningFactors.end();
        }

        cv::Mat to_gray_if_needed(const cv::Mat &src) {
            if (src.channels() == 1)
                return src;
            cv::Mat gray;
            cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
            return gray;
        }

        cv::Mat apply_sum_binning(const cv::Mat &src_gray, const int factor) {
            if (factor == 1)
                return src_gray;

            if (src_gray.empty())
                throw std::logic_error("binning failed: empty frame");

            if (src_gray.channels() != 1)
                throw std::logic_error("binning failed: only single-channel frame is supported");

            if (src_gray.cols % factor != 0 || src_gray.rows % factor != 0)
                throw std::logic_error("binning failed: frame size is not divisible by factor");

            const int out_width = src_gray.cols / factor;
            const int out_height = src_gray.rows / factor;

            cv::Mat src_i32;
            src_gray.convertTo(src_i32, CV_32SC1);
            cv::Mat binned_i32(out_height, out_width, CV_32SC1, cv::Scalar(0));

            for (int out_y = 0; out_y < out_height; ++out_y) {
                const int in_y = out_y * factor;
                for (int out_x = 0; out_x < out_width; ++out_x) {
                    const int in_x = out_x * factor;
                    int sum_value = 0;
                    for (int dy = 0; dy < factor; ++dy) {
                        const int *row_ptr = src_i32.ptr<int>(in_y + dy) + in_x;
                        for (int dx = 0; dx < factor; ++dx)
                            sum_value += row_ptr[dx];
                    }
                    binned_i32.at<int>(out_y, out_x) = sum_value;
                }
            }

            return binned_i32;
        }

        void scale_measurements_to_original_frame(std::vector<TOptionsMeasurement> &meas, const int factor) {
            if (factor == 1)
                return;

            for (auto &value: meas) {
                value.x_weight *= factor;
                value.y_weight *= factor;
                value.x_rec *= factor;
                value.y_rec *= factor;
                value.rec_width *= factor;
                value.rec_height *= factor;
            }
        }

        void scale_draw_data_to_original_frame(std::vector<TDrawMeasurement> &draw_data, const int factor) {
            if (factor == 1)
                return;

            for (auto &value: draw_data) {
                value.rect.x *= factor;
                value.rect.y *= factor;
                value.rect.width *= factor;
                value.rect.height *= factor;

                value.box.center.x *= factor;
                value.box.center.y *= factor;
                value.box.size.width *= factor;
                value.box.size.height *= factor;
            }
        }
    }

	void init_fr_proc_logger(int cam_index) {
		logger = log4cxx::Logger::getLogger("dp1-" + std::to_string(cam_index) + ".fr-proc");
		init_dp1_multi_th_proc_logger(cam_index);
	}

	class fr_proc_impl : public frame_processor {
		size_t nFrames_{};
		//bool update_bg_model = false;//true  false
		int frame_width_;
		int frame_height_;
        int proc_frame_width_;
        int proc_frame_height_;
        int binning_factor_;

		TDataproConfig data_param_;
		TDataproVar var_;
		std::vector<TOptionsMeasurement> out_meas_;
		std::deque<frame_n_header> rc_frames_;

        TDataRes data_res;

        cv::VideoWriter video;

        std::ofstream file_bin;
        std::time_t t_file{};
        std::chrono::time_point<std::chrono::system_clock> time_start = startTime();//std::chrono::system_clock::now();

		int64 t0_{}, tp0_{}, t1_{};
		int64 processingTime_ = 0;
		const int NumberFrame_ = 10;

		const prg_config &cfg_;

        MedianFilter filter;
        MedianFilter2 filter2;
		int cam_index_;

        const TDataCalibrationCamera &cam_param_;
        TDataCalibrationFrame frame_param_;
//        cv::Mat R_matrix_, t_vec_, r_vec_, descriptors_scene_;
//        std::vector<cv::Point2f> chessboard_corners_;
//        std::vector<cv::KeyPoint> keypoints_scene_;

        TFolder folder_name_dp1;
        std::string file_name;
        std::time_t file_time{};
        cv::Mat KernelGauss, KernelStroke;

		std::unique_ptr<i_calc_tile_limit> calc_limiter;

		void init_params();

	public:
        fr_proc_impl(const prg_config &cfg, const TDataCalibrationCamera &cam_cfg, int frame_width, int frame_height, int cam_index) : frame_width_(frame_width),
																				 frame_height_(frame_height),
                                                                                                     proc_frame_width_(frame_width),
                                                                                                     proc_frame_height_(frame_height),
                                                                                                     binning_factor_(1),
																				 cfg_(cfg),
                                                                                 filter(cfg_.median_bg.num_frame * cfg_.median_bg.sampling_period),
                                                                                 filter2(cfg_.median_bg.num_frame, cfg_.median_bg.sampling_period, true, false, MedianFilter2::OutputMode::DIFFERENCE),
																				 cam_index_{cam_index},
																				 cam_param_{cam_cfg}{

			init_params();
		}
        ~fr_proc_impl() override {// TODO
			LOG4CXX_INFO(logger, "enter");

            if (cfg_.test.display.display) {
                cv::destroyAllWindows();
            }
            if (cfg_.test.display.video_save) {
                video.release();
            }
            if (cfg_.test.res_file.switched) {
                file_bin.close();
            }
        }

        void proc_next_frame(frame_n_header rc_frame) override;
	};

	static std::unique_ptr<fr_proc_impl> frame_proc;

	std::unique_ptr<frame_processor> get_fr_processor(const prg_config &cfg, const TDataCalibrationCamera &cam_cfg, int frame_width, int frame_height, int cam_index) {
		return std::make_unique<fr_proc_impl>(cfg, cam_cfg, frame_width, frame_height, cam_index);
	}

	void fr_proc_impl::init_params() {
		//int fps = cap.get(cv::CAP_PROP_FPS);

		LOG4CXX_INFO(logger, "Reading DataPro1 configuration file parameters.");
		//getConfig(cfg_,frame_width_, frame_height_, config_);

		
        if (cfg_.filters.corr.switched) {
            createKernelGaussCov(cfg_.filters.corr.height, KernelGauss);
        }
		if (cfg_.test.display.display) {
			LOG4CXX_INFO(logger, "Display enabled.");
			cv::namedWindow("Image", cv::WINDOW_NORMAL);
		}

        if (cfg_.test.display.video_save) {
            LOG4CXX_INFO(logger, "Video recording enabled.");
            if (!std::filesystem::exists(cfg_.test.out_folder)) {
                std::filesystem::create_directories(cfg_.test.out_folder);
            }
            video.open(cfg_.test.out_folder + "/" + cfg_.test.display.video_name, cv::VideoWriter::fourcc('a', 'v', 'c', '1'), cfg_.test.display.fps, cv::Size(frame_width_, frame_height_));
        }

        if(cfg_.binocular.switched){
            fillCameraRt(cfg_.binocular.R_matrix,
                         cfg_.binocular.t_vector,
                         frame_param_.R_matrix,
                         frame_param_.t_vec);
            cv::Rodrigues(frame_param_.R_matrix, frame_param_.r_vec);
            if (!cfg_.binocular.using_template){
                LOG4CXX_INFO(logger, "Given camera orientation and position are used.");
            } else{
                LOG4CXX_INFO(logger, "Camera orientation and position are calculated for each frame.");
            }
        }

        if (!is_valid_binning_factor(cfg_.binning.factor)) {
            throw std::logic_error("unsupported binning.factor in runtime: " + std::to_string(cfg_.binning.factor));
        }

        if (cfg_.binning.mode != kBinningModeSum) {
            throw std::logic_error("unsupported binning.mode in runtime: " + cfg_.binning.mode);
        }

        binning_factor_ = (cfg_.binning.switched) ? cfg_.binning.factor : 1;
        if (binning_factor_ > 1) {
            if (frame_width_ % binning_factor_ != 0 || frame_height_ % binning_factor_ != 0) {
                throw std::logic_error("frame size must be divisible by binning.factor");
            }
            proc_frame_width_ = frame_width_ / binning_factor_;
            proc_frame_height_ = frame_height_ / binning_factor_;
            LOG4CXX_INFO(logger, "Binning enabled, factor=" << binning_factor_ << ", mode=" << cfg_.binning.mode);
        }

		LOG4CXX_INFO(logger, "Calculation of DataPro1 parameters.");
        initializingParamDatapro1(cfg_.subtractor, cfg_.multiproc.tiles_factor, cfg_.def_border, proc_frame_width_, proc_frame_height_,
								  cfg_.filters.corr, cfg_.filters.matched, cfg_.filters.blur, data_param_, var_);
		calc_limiter = get_tile_calc_limiter(cfg_.calc_tile_lim_cfg, data_param_.numFragX, data_param_.numFragY);
		t0_ = cv::getTickCount();
	}

	void fr_proc_impl::proc_next_frame(frame_n_header rc_frame) {
		rc_frames_.push_front(rc_frame);
		if (rc_frames_.size() > cfg_.num_frame_to_keep)
			rc_frames_.pop_back();

        cv::Mat med_im, convert_im;

        if (cfg_.source.source=="campro"){
            data_res.data_cam.cam_index = cam_index_;
            data_res.data_frame.index_frame = rc_frames_.front().cam_pro_header->index;
            data_res.data_frame.height = rc_frames_.front().cam_pro_header->height;
            data_res.data_frame.width = rc_frames_.front().cam_pro_header->width;
            data_res.data_frame.Az = rc_frames_.front().cam_pro_header->Azimuth;
            data_res.data_frame.El = rc_frames_.front().cam_pro_header->Elevation;
            data_res.data_frame.V_az = rc_frames_.front().cam_pro_header->AzimuthSpeed;
            data_res.data_frame.V_el = rc_frames_.front().cam_pro_header->ElevationSpeed;
			data_res.data_frame.turretInfoValid = rc_frames_.front().cam_pro_header->turretInfoValid;
            data_res.data_frame.exposureStart = rc_frames_.front().cam_pro_header->exposureStart;
            data_res.data_frame.exposureLength = rc_frames_.front().cam_pro_header->exposureLength;
            data_res.data_frame.pixelHeight = rc_frames_.front().cam_pro_header->pixelHeight;
            data_res.data_frame.pixelWidth = rc_frames_.front().cam_pro_header->pixelWidth;
            data_res.data_frame.focalLength = rc_frames_.front().cam_pro_header->focalLength;
        }
        else{
            data_res.data_cam.cam_index = cam_index_;
            data_res.data_frame.index_frame = static_cast<int>(nFrames_);
            data_res.data_frame.height = rc_frames_.front().mat->rows;
            data_res.data_frame.width = rc_frames_.front().mat->cols;
            data_res.data_frame.Az = 0;
            data_res.data_frame.El = 0;
            data_res.data_frame.V_az = 1;
            data_res.data_frame.V_el = 1;
			data_res.data_frame.turretInfoValid = true;
            data_res.data_frame.exposureStart = time_start + std::chrono::milliseconds(cfg_.source.frame_period)*static_cast<int>(nFrames_); //40ms*static_cast<int>(nFrames_);
            data_res.data_frame.exposureLength = 1;
            data_res.data_frame.pixelHeight = 5E-6;
            data_res.data_frame.pixelWidth = 5E-6;
            data_res.data_frame.focalLength = 0.5;
        }

        nFrames_++;
		if (nFrames_ % NumberFrame_ == 0) {
			t1_ = cv::getTickCount();
            LOG4CXX_INFO(logger, "\nFrames captured: " + cv::format("%5lld", (long long int) nFrames_) +
                    "    Average FPS: " + cv::format("%9.1f",
                                                     (double) cv::getTickFrequency() * NumberFrame_ / static_cast<double>(t1_ - t0_)) +
                    "    Average time per frame: " + cv::format("%9.2f ms",
                                                                (double) (t1_ - t0_) * 1000.0f / (NumberFrame_ * cv::getTickFrequency())) +
                    "    Average processing time: " + cv::format("%9.2f ms", (double) (processingTime_) * 1000.0f /
                                                                             (NumberFrame_ *
                                                                              cv::getTickFrequency())));
			t0_ = t1_;
			processingTime_ = 0;
		}
        //cv::setNumThreads(24);
        tp0_ = cv::getTickCount();

        file_time = std::time(nullptr);
        creating_file_name(file_time, data_res.data_cam.cam_index,
                           data_res.data_frame.index_frame, file_name);

        if(cfg_.binocular.switched){
            if (cfg_.binocular.using_template){
                (rc_frames_.front().mat)->convertTo(convert_im, CV_8UC1, 1.0 / 256, 0);
                auto found = poseEstimationFromCoplanarPoints(convert_im, cam_param_, frame_param_);
                if (!found){
                    LOG4CXX_INFO(logger, "Template is not defined on the frame. Given orientation and position are used.");
                    fillCameraRt(cfg_.binocular.R_matrix,
                                 cfg_.binocular.t_vector,
                                 frame_param_.R_matrix,
                                 frame_param_.t_vec);
                    cv::Rodrigues(frame_param_.R_matrix, frame_param_.r_vec);
                }
            }
        }

        cv::Mat frame_to_process = *(rc_frames_.front().mat);
        if (binning_factor_ > 1) {
            const cv::Mat gray_frame = to_gray_if_needed(frame_to_process);
            frame_to_process = apply_sum_binning(gray_frame, binning_factor_);
        }

        datapro1(frame_to_process, var_, data_param_,
                 cfg_.segment, file_name, cfg_.test, folder_name_dp1, //cfg_.frame,
                 cfg_.filters, KernelGauss, out_meas_, calc_limiter.get(), cfg_.multiproc.dp1_num_thread);
        scale_measurements_to_original_frame(out_meas_, binning_factor_);
        scale_draw_data_to_original_frame(var_.data_draw, binning_factor_);
        processingTime_ += cv::getTickCount() - tp0_;

        bool median = false;
        if(cfg_.median_bg.switched){//TODO
            cv::namedWindow("Image Mediana", cv::WINDOW_NORMAL);
            rc_frames_.front().mat->convertTo(convert_im, CV_8UC1, 1.0 / 256, 0);
            if (median){
                if (nFrames_ == 1)
                    filter.init_median_lists(convert_im.total());
                med_im = filter.process_frame(convert_im);
                med_im = cv::abs(convert_im - med_im);
            }
            else {
                if (nFrames_ == 1)
                    filter2.init_buffer(convert_im);
                med_im = filter2.process_frame(convert_im);
            }

            threshold(med_im, med_im, 50, 256, 0);
            dilate(med_im, med_im, cv::getStructuringElement(cv::MORPH_ELLIPSE,
                                                             cv::Size(3, 3)),
                   cv::Point(-1, -1), 3);
            cv::resize(med_im, med_im, cv::Size(), cfg_.test.display.scale, cfg_.test.display.scale);
            cv::imshow("Image Mediana", med_im);
        }

        data_res.meas = out_meas_;
//        data_res.calib_cam = cam_param_;
        data_res.calib_frame = frame_param_;
		data_res.data_frame.dp1_spent_time = duration_cast<milliseconds>(steady_clock::now() - rc_frame.ipc_start_time).count();
		LOG4CXX_INFO(logger, "full frame proc time (ms): " << data_res.data_frame.dp1_spent_time);

		calc_limiter->set_last_frame_proc_time_mc(data_res.data_frame.dp1_spent_time);
		send_res_to_dp2(data_res);

        if (cfg_.test.res_file.switched){
            if(cfg_.test.res_file.one_file)
                save_res(out_meas_, data_res.data_frame, cfg_.test.out_folder, data_res.data_cam,cfg_.test.res_file.time_interval,
                         file_bin, folder_name_dp1, t_file);
            else
                save_res(out_meas_, data_res.data_frame, cfg_.test.out_folder, data_res.data_cam, cfg_.test.res_file.txt_file,
                         cam_param_, frame_param_,
                         folder_name_dp1, file_name,cfg_.binocular.switched,file_bin);
        }

        if (cfg_.test.display.display||cfg_.test.display.video_save) {
            cv::Mat frame_show,frame_show_resize,frame_show_3D;
			showing(*(rc_frames_.front().mat), var_.data_draw, cfg_.test.display.display_marks,
                    cfg_.test.display.rotayte_rec, frame_show);
            if(cfg_.test.display.display) {
                if(cfg_.binocular.switched){
                    frame_show.copyTo(frame_show_3D);
                    cv::drawFrameAxes(frame_show_3D, cam_param_.cameraMatrix, cam_param_.distCoeffs,
                                      frame_param_.r_vec, frame_param_.t_vec, 6*cam_param_.square_size);
//                    cv::drawChessboardCorners(frame_show_3D, cv::Size(cam_param_.board_height,cam_param_.board_width), frame_param_.chessboard_corners, true);
                    cv::resize(frame_show_3D, frame_show_resize, cv::Size(), cfg_.test.display.scale, cfg_.test.display.scale);
                }
                else
                    cv::resize(frame_show, frame_show_resize, cv::Size(), cfg_.test.display.scale, cfg_.test.display.scale);
                cv::imshow("Image", frame_show_resize);
            }
            if(cfg_.test.display.video_save)
                video.write(frame_show);
		}

	}
}
