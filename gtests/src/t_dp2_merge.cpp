//
// Created by u on 19.10.24.
//
#include <gtest/gtest.h>
#include "datapro2/src/datapro2.h"

TEST(test_dp2, merge_traj) {
	StrobeMethod processor(0.0,    //config.min_speed,
						   250.0,    //config.max_speed,
						   3,    //config.max_ptt_frames,
						   10,    //config.max_tracking_frames,
						   10,    //config.trajectory_drop_max_confirmed_frames,
						   8,    //config.trajectory_drop_min_confirmed_frames,
						   0.0,    //config.trajectory_drop_min_speed,
						   250.0,    //config.trajectory_drop_max_speed,
						   0.0,    //config.trajectory_drop_min_acceleration,
						   50.0,    //config.trajectory_drop_max_acceleration,
						   100.0,    //config.trajectory_drop_max_x_std_dev,
						   0.03,    //config.trajectory_drop_max_y_std_dev,
						   0.03,    //config.trajectory_drop_max_z_std_dev,
						   100.0,    //config.trajectory_drop_max_xyz_std_dev,
						   10,    //config.trajectory_show_max_confirmed_frames,
						   8,    //config.trajectory_show_min_confirmed_frames,
						   0.0,    //config.trajectory_show_min_speed,
						   250.0,    //config.trajectory_show_max_speed,
						   0.0,    //config.trajectory_show_min_acceleration,
						   50.0,    //config.trajectory_show_max_acceleration,
						   100.0,    //config.trajectory_show_max_x_std_dev,
						   0.03,    //config.trajectory_show_max_y_std_dev,
						   0.03,    //config.trajectory_show_max_z_std_dev,
						   100.0,    //config.trajectory_show_max_xyz_std_dev,
						   3,    //config.trajectory_show_min_frames,
						   300,    //config.noisy_region_bounding_box_filter_size,
						   3000,    //config.noisy_region_bounding_box_max_objects_limit,
						   "test_out_dp2",    //config.dp2_out_folder,
						   false,    //config.create_text_file_results,
						   false,    //config.create_binary_file_results,
						   false,    //config.create_json_file_results,
						   false,    //config.linear_model,
						   10.0,    //config.k_sigma,
						   1.0,		//std_measurement
//						   1.7754558962424419e+03,    //camera_config.focal_length_x,
//						   1.7774826297767709e+03,    //camera_config.focal_length_y,
//						   9.9139890322802023e+02,    //camera_config.optical_centers_x,
//						   5.0080258652647649e+02    //camera_config.optical_centers_y
                           true     // binocular.use_corners
	);
	TDataCam data_cam{7};
	TDataFrame data_frame{};
	data_frame.width = 2000;
	data_frame.height = 1000;
    TDataCalibrationCamera cam_cfg{};
    cam_cfg.cameraMatrix.at<double>(0,0) = 700;
    cam_cfg.cameraMatrix.at<double>(1,1) = 700;
    cam_cfg.cameraMatrix.at<double>(0,2) = data_frame.width / 2.0;
    cam_cfg.cameraMatrix.at<double>(1,2) = data_frame.height / 2.0;
	int frame_index{};
	int obj_index{};
	auto frame_interval = std::chrono::milliseconds(500);
	for (int iter = 0; iter < 2; ++iter) {
		for (int i = 1; i <= 20; ++i) {
			data_frame.index_frame = frame_index++;
			data_frame.exposureStart += frame_interval;
			std::vector<Measurement> measurements{
					{++obj_index, 0.0, 0.0, 1, 100.0 + i * 50.0, 500.0, 10.0, data_frame.exposureStart, 0.0, 0.0}};
			processor.process_frame(data_cam, data_frame, measurements, cam_cfg);
			if(iter == 0){
				if(i == 1) {
					EXPECT_EQ(processor.get_tr_number(), 0);
					EXPECT_EQ(processor.get_ppt_number(), 1);
				}else{
					EXPECT_EQ(processor.get_tr_number(), 1);
					EXPECT_EQ(processor.get_ppt_number(), 0);
				}
			}else{
				EXPECT_EQ(processor.get_tr_number(), 1);
				if(i == 1) {
					EXPECT_EQ(processor.get_ppt_number(), 1);
				}else{
					EXPECT_EQ(processor.get_ppt_number(), 0);
				}
			}
		}
		for (int i = 19; i >= 0; --i) {
			data_frame.index_frame = frame_index++;
			data_frame.exposureStart += frame_interval;
			std::vector<Measurement> measurements{
					{++obj_index, 0.0, 0.0, 1, 100.0 + i * 50.0, 500.0, 10.0, data_frame.exposureStart, 0.0, 0.0}};
			processor.process_frame(data_cam, data_frame, measurements, cam_cfg);

			EXPECT_EQ(processor.get_tr_number(), 1);
			if (i == 19) {
				EXPECT_EQ(processor.get_ppt_number(), 1);
			}else{
				EXPECT_EQ(processor.get_ppt_number(), 0);
			}
		}
	}
}

TEST(test_dp2, third_measure_quadric_model) {
	StrobeMethod processor(0.0,    //config.min_speed,
						   250.0,    //config.max_speed,
						   3,    //config.max_ptt_frames,
						   10,    //config.max_tracking_frames,
						   10,    //config.trajectory_drop_max_confirmed_frames,
						   8,    //config.trajectory_drop_min_confirmed_frames,
						   0.0,    //config.trajectory_drop_min_speed,
						   250.0,    //config.trajectory_drop_max_speed,
						   0.0,    //config.trajectory_drop_min_acceleration,
						   50.0,    //config.trajectory_drop_max_acceleration,
						   100.0,    //config.trajectory_drop_max_x_std_dev,
						   0.03,    //config.trajectory_drop_max_y_std_dev,
						   0.03,    //config.trajectory_drop_max_z_std_dev,
						   100.0,    //config.trajectory_drop_max_xyz_std_dev,
						   10,    //config.trajectory_show_max_confirmed_frames,
						   8,    //config.trajectory_show_min_confirmed_frames,
						   0.0,    //config.trajectory_show_min_speed,
						   250.0,    //config.trajectory_show_max_speed,
						   0.0,    //config.trajectory_show_min_acceleration,
						   50.0,    //config.trajectory_show_max_acceleration,
						   100.0,    //config.trajectory_show_max_x_std_dev,
						   0.03,    //config.trajectory_show_max_y_std_dev,
						   0.03,    //config.trajectory_show_max_z_std_dev,
						   100.0,    //config.trajectory_show_max_xyz_std_dev,
						   3,    //config.trajectory_show_min_frames,
						   300,    //config.noisy_region_bounding_box_filter_size,
						   3000,    //config.noisy_region_bounding_box_max_objects_limit,
						   "test_out_dp2",    //config.dp2_out_folder,
						   false,    //config.create_text_file_results,
						   false,    //config.create_binary_file_results,
						   false,    //config.create_json_file_results,
						   false,    //config.linear_model,
						   10.0,    //config.k_sigma,
						   1.0,		//std_measurement
//						   1.7754558962424419e+03,    //camera_config.focal_length_x,
//						   1.7774826297767709e+03,    //camera_config.focal_length_y,
//						   9.9139890322802023e+02,    //camera_config.optical_centers_x,
//						   5.0080258652647649e+02    //camera_config.optical_centers_y
						   true     // binocular.use_corners
	);
	TDataCam data_cam{7};
	TDataFrame data_frame{};
	data_frame.width = 2000;
	data_frame.height = 1000;
    TDataCalibrationCamera cam_cfg{};
    cam_cfg.cameraMatrix.at<double>(0,0) = 700;
    cam_cfg.cameraMatrix.at<double>(1,1) = 700;
    cam_cfg.cameraMatrix.at<double>(0,2) = data_frame.width / 2.0;
    cam_cfg.cameraMatrix.at<double>(1,2) = data_frame.height / 2.0;
    int frame_index{};
	int obj_index{};
	auto frame_interval = std::chrono::milliseconds(500);

	{
		data_frame.index_frame = frame_index++;
		data_frame.exposureStart += frame_interval;
		std::vector<Measurement> measurements{
				{++obj_index, 0.0, 0.0, 1, 100.0 + frame_index * 50.0, 500.0, 10.0, data_frame.exposureStart, 0.0,
				 0.0}};
		processor.process_frame(data_cam, data_frame, measurements, cam_cfg);
		EXPECT_EQ(processor.get_tr_number(), 0);
		EXPECT_EQ(processor.get_ppt_number(), 1);
	}

	{
		data_frame.index_frame = frame_index++;
		data_frame.exposureStart += frame_interval;
		std::vector<Measurement> measurements{
				{++obj_index, 0.0, 0.0, 1, 100.0 + frame_index * 50.0, 500.0, 10.0, data_frame.exposureStart, 0.0,
				 0.0}};
		processor.process_frame(data_cam, data_frame, measurements, cam_cfg);
		EXPECT_EQ(processor.get_tr_number(), 1);
		EXPECT_EQ(processor.get_ppt_number(), 0);
	}

	{
		data_frame.index_frame = frame_index++;
		data_frame.exposureStart += frame_interval;
		std::vector<Measurement> measurements{
				{++obj_index, 0.0, 0.0, 1, 100.0 + frame_index * 50.0, 500.0, 10.0, data_frame.exposureStart, 0.0,
				 0.0}};
		processor.process_frame(data_cam, data_frame, measurements, cam_cfg);
		EXPECT_EQ(processor.get_tr_number(), 1);
		EXPECT_EQ(processor.get_ppt_number(), 0);
	}
}
