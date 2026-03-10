#include <gtest/gtest.h>
#include <numeric>
#include <random>
#include "m_rpc_sink.h"
#include "m_rpc_d_former.h"
#include "mem_store.h"
#include "datapro1/src/datarpoTypes.h"
#include "datapro1/src/dp1_rpc_data_mrsh.h"
#include "datapro2/src/dp2_rpc_data_mrsh.h"
#include "datapro2/src/datapro2Types.h"

using std::vector;

class test_rpc_sink : public rpc_sink {
public:
	void on_rd_msg_complite(const uint8_t *data, std::size_t len) override {
		++num_read_msg;
		last_msg_size = len;
		last_msg_sum = std::accumulate(data, data + len, 0);
	}

	int num_read_msg{};
	std::size_t last_msg_size{};
	int last_msg_sum{};
};

TEST(test_rpc, sink_general) {
	test_rpc_sink sink;
	bool need_break{};
	{
		uint8_t test_data[] = {1, 0, 0, 0,
							   1,
							   10, 0, 0, 0,
							   0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
		sink.on_next_raw_read(test_data, sizeof(test_data), need_break);
		EXPECT_EQ(sink.num_read_msg, 1);
		EXPECT_EQ(sink.last_msg_size, 10);
		EXPECT_EQ(sink.last_msg_sum, 45);
	}
	{    //2nd msg - heart-beat
		uint8_t test_data[] = {2, 0, 0, 0,
							   0};
		sink.on_next_raw_read(test_data, sizeof(test_data), need_break);
		EXPECT_EQ(sink.num_read_msg, 1);
		EXPECT_EQ(sink.last_msg_size, 10);
	}
	{    //3-th message, by parts
		uint8_t test_data[] = {3, 0, 0, 0,
							   1,
							   11, 0, 0, 0,
							   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		for (uint8_t el: test_data) {
			sink.on_next_raw_read(&el, 1, need_break);
		}
		EXPECT_EQ(sink.num_read_msg, 2);
		EXPECT_EQ(sink.last_msg_size, 11);
		EXPECT_EQ(sink.last_msg_sum, 55);
	}
}


class test_rpc_sink_store_last_msg : public rpc_sink {
public:
	void on_rd_msg_complite(const uint8_t *data, std::size_t len) override {
		last_msg_ = std::vector(data, data + len);
	}

	std::vector<uint8_t> last_msg_;
};

static void test_sink_n_former(rpc_data_former &data_former, test_rpc_sink_store_last_msg &sink, int data_len) {
	static std::random_device rd;
	static auto mtgen = std::mt19937{rd()};
	static auto ud = std::uniform_int_distribution<>{0, 255};
	vector<uint8_t> gen_data;
	gen_data.reserve(data_len);
	for (int i = 0; i < data_len; ++i)
		gen_data.push_back(ud(mtgen));
	auto raw_msg = data_former.form_next_msg(gen_data.data(), gen_data.size());

	bool break_conn{false};
	sink.on_next_raw_read(raw_msg.data(), raw_msg.size(), break_conn);
	EXPECT_EQ(sink.last_msg_, gen_data);
	EXPECT_FALSE(break_conn);

	//check for heart-beat
	if(!(data_len % 34)){
		raw_msg = data_former.check_heart_beat(true);
		sink.on_next_raw_read(raw_msg.data(), raw_msg.size(), break_conn);
		EXPECT_FALSE(break_conn);
	}
}

TEST(test_rpc, sink_plus_former) {
	rpc_data_former data_former;
	test_rpc_sink_store_last_msg sink;

	for (int iter = 0; iter < 5; ++iter)
		for (int len = 0; len < 2048; ++len)
			test_sink_n_former(data_former, sink, len);

}

static void compare_2_matrix(const cv::Mat &m1, const cv::Mat &m2){
	EXPECT_EQ(m1.rows, m2.rows);
	EXPECT_EQ(m1.cols, m2.cols);
	EXPECT_EQ(m1.elemSize(), m2.elemSize());
	EXPECT_EQ(m1.type(), m2.type());

	bool isEqual = (sum(m1 != m2) == cv::Scalar(0,0,0,0));
	EXPECT_TRUE(isEqual);
}

TEST(test_rpc, cv_mat_serialize_2d_double) {
	cv::Mat test_matrix(2, 3, CV_64FC1);
	test_matrix.at<double>(0,0) = 0.0;
	test_matrix.at<double>(0,1) = 0.1;
	test_matrix.at<double>(0,2) = 0.2;
	test_matrix.at<double>(1,0) = 1.0;
	test_matrix.at<double>(1,1) = 1.1;
	test_matrix.at<double>(1,2) = 1.2;
	CMemStore ms;
	serialize_Mat(ms, test_matrix);
	CMemStore rest_ms(ms.data(), ms.size());
	cv::Mat rest_matrix;
	deserialize_Mat(rest_ms, rest_matrix);

	compare_2_matrix(test_matrix, rest_matrix);
}

TEST(test_rpc, camera_calibration_data_serialize){
	TDataCalibrationCamera cam_data;

	cam_data.cameraMatrix = cv::Mat(5, 10, CV_64FC1);
	cam_data.cameraMatrix.at<double>(2,3) = 2.3;

	cam_data.distCoeffs = cv::Mat(7, 15, CV_32F);
	cam_data.distCoeffs.at<float>(4,5) = 4.5;

	cam_data.avg_reprojection_error = 123.45;
	cam_data.square_size = 23.4f;
	cam_data.board_width = 123;
	cam_data.board_height = 456;

	CMemStore ms;
	serialize_camera_calibration_data(ms, cam_data);
	CMemStore rest_ms(ms.data(), ms.size());

	TDataCalibrationCamera rest_data;
	deserialize_camera_calibration_data(rest_ms, rest_data);

	compare_2_matrix(cam_data.cameraMatrix, rest_data.cameraMatrix);
	compare_2_matrix(cam_data.distCoeffs, rest_data.distCoeffs);
	EXPECT_EQ(cam_data.avg_reprojection_error, rest_data.avg_reprojection_error);
	EXPECT_EQ(cam_data.square_size, rest_data.square_size);
	EXPECT_EQ(cam_data.board_width, rest_data.board_width);
	EXPECT_EQ(cam_data.board_height, rest_data.board_height);
}

TEST(test_rpc, frame_calibration_data_serialize) {
	TDataCalibrationFrame frame_data;
	frame_data.R_matrix = cv::Mat(5, 10, CV_64FC1);
	frame_data.R_matrix.at<double>(2,3) = 2.3;

	frame_data.t_vec = cv::Mat(7, 15, CV_32F);
	frame_data.t_vec.at<float>(4,5) = 4.5;

	frame_data.r_vec = cv::Mat(6, 8, CV_32S);
	frame_data.r_vec.at<int32_t>(4,5) = -2345;

	frame_data.descriptors_scene = cv::Mat(4, 9, CV_16U);
	frame_data.descriptors_scene.at<int32_t>(3,6) = 16386;

	frame_data.chessboard_corners.resize(123);
	frame_data.chessboard_corners[2].x = 123;
	frame_data.chessboard_corners[2].y = 321;

	frame_data.keypoints_scene.resize(32);
	frame_data.keypoints_scene[30].angle = 20.345;
	frame_data.keypoints_scene[31].pt.y = 32.851;

	CMemStore ms;
	serialize_frame_calibration_data(ms, frame_data);
	CMemStore rest_ms(ms.data(), ms.size());

	TDataCalibrationFrame rest_data;
	deserialize_frame_calibration_data(rest_ms, rest_data);
	compare_2_matrix(frame_data.R_matrix, rest_data.R_matrix);
	compare_2_matrix(frame_data.t_vec, rest_data.t_vec);
	compare_2_matrix(frame_data.r_vec, rest_data.r_vec);
	compare_2_matrix(frame_data.descriptors_scene, rest_data.descriptors_scene);
	EXPECT_TRUE(frame_data.chessboard_corners == rest_data.chessboard_corners);
	EXPECT_EQ(frame_data.keypoints_scene.size(), rest_data.keypoints_scene.size());
	for(unsigned int i = 0; i < frame_data.keypoints_scene.size(); ++i){
		const auto &kp1 = frame_data.keypoints_scene[i];
		const auto &kp2 = rest_data.keypoints_scene[i];
		EXPECT_EQ(kp1.pt, kp2.pt);
		EXPECT_EQ(kp1.size, kp2.size);
		EXPECT_EQ(kp1.angle, kp2.angle);
		EXPECT_EQ(kp1.response, kp2.response);
		EXPECT_EQ(kp1.octave, kp2.octave);
		EXPECT_EQ(kp1.class_id, kp2.class_id);
	}
}

TEST(test_rpc, dp1_res_data_serialize) {
	CMemStore ms;
	TDataRes src_data;
	src_data.data_cam.cam_index = 43;
	src_data.data_cam.Xcam = 1001;
	src_data.data_cam.Ycam = 1002;
	src_data.data_cam.Zcam = 1003;

	src_data.data_frame.index_frame = 2001;
	src_data.data_frame.dp1_spent_time = 200101;
	src_data.data_frame.exposureStart = std::chrono::system_clock::now();///
	src_data.data_frame.width = 2003;///
	src_data.data_frame.height = 2004;///
	src_data.data_frame.exposureLength = 2005;   // s////
	src_data.data_frame.focalLength = 2006;      // m///
	src_data.data_frame.pixelWidth = 2007;       // m///
	src_data.data_frame.pixelHeight = 2008;      // m///
	src_data.data_frame.El = 2009;///
	src_data.data_frame.Az = 2010;///
	src_data.data_frame.V_el = 2011;///
	src_data.data_frame.V_az = 2012;///
	src_data.data_frame.turretInfoValid = true;///

	src_data.meas.push_back({1, 2, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0});
	src_data.meas.push_back({51, 52, 53.0, 54.0, 55.0, 56.0, 57.0, 58.0, 59.0, 510.0, 511.0});

	serialize_dp1_res(ms, src_data);

	CMemStore rest_ms(ms.data(), ms.size());
	TDataRes rest_data;
	deserialize_dp1_res(rest_ms, rest_data);
	EXPECT_EQ(src_data.data_cam.cam_index, rest_data.data_cam.cam_index);
	EXPECT_EQ(src_data.data_cam.Zcam, rest_data.data_cam.Zcam);

	EXPECT_EQ(src_data.data_frame.index_frame, rest_data.data_frame.index_frame);
	EXPECT_EQ(src_data.data_frame.dp1_spent_time, rest_data.data_frame.dp1_spent_time);
	EXPECT_EQ(src_data.data_frame.V_az, rest_data.data_frame.V_az);
	EXPECT_EQ(src_data.data_frame.turretInfoValid, rest_data.data_frame.turretInfoValid);	//and last member

	EXPECT_EQ(src_data.meas.size(), rest_data.meas.size());
	EXPECT_EQ(src_data.meas[0].id_obj, rest_data.meas[0].id_obj);
	EXPECT_EQ(src_data.meas[0].num_pix_obj, rest_data.meas[0].num_pix_obj);
	EXPECT_EQ(src_data.meas[0].x_weight, rest_data.meas[0].x_weight);
	EXPECT_EQ(src_data.meas[0].y_weight, rest_data.meas[0].y_weight);
	EXPECT_EQ(src_data.meas[0].rec_width, rest_data.meas[0].rec_width);
	EXPECT_EQ(src_data.meas[0].rec_height, rest_data.meas[0].rec_height);
	EXPECT_EQ(src_data.meas[0].mean_brightness_obj, rest_data.meas[0].mean_brightness_obj);
	EXPECT_EQ(src_data.meas[0].std_brightness_obj, rest_data.meas[0].std_brightness_obj);
	EXPECT_EQ(src_data.meas[0].mean_brightness_im, rest_data.meas[0].mean_brightness_im);
	EXPECT_EQ(src_data.meas[0].std_brightness_im, rest_data.meas[0].std_brightness_im);
	EXPECT_EQ(src_data.meas[0].obj_angel, rest_data.meas[0].obj_angel);

	EXPECT_EQ(src_data.meas[1].id_obj, rest_data.meas[1].id_obj);
	EXPECT_EQ(src_data.meas[1].num_pix_obj, rest_data.meas[1].num_pix_obj);
	EXPECT_EQ(src_data.meas[1].x_weight, rest_data.meas[1].x_weight);
	EXPECT_EQ(src_data.meas[1].y_weight, rest_data.meas[1].y_weight);
	EXPECT_EQ(src_data.meas[1].rec_width, rest_data.meas[1].rec_width);
	EXPECT_EQ(src_data.meas[1].rec_height, rest_data.meas[1].rec_height);
	EXPECT_EQ(src_data.meas[1].mean_brightness_obj, rest_data.meas[1].mean_brightness_obj);
	EXPECT_EQ(src_data.meas[1].std_brightness_obj, rest_data.meas[1].std_brightness_obj);
	EXPECT_EQ(src_data.meas[1].mean_brightness_im, rest_data.meas[1].mean_brightness_im);
	EXPECT_EQ(src_data.meas[1].std_brightness_im, rest_data.meas[1].std_brightness_im);
	EXPECT_EQ(src_data.meas[1].obj_angel, rest_data.meas[1].obj_angel);
}

TEST(test_rpc, dp2_res_data_serialize) {
	Trajectory orig_track;
	orig_track.measurements.push_back({100, 101, 102, 103, 104.0, 105.0, 106.0, std::chrono::system_clock::now()});
	orig_track.measurements.push_back({200, 201, 202, 203, 204.0, 205.0, 206.0, std::chrono::system_clock::now()});
	orig_track.measurements.push_back({1200, 1201, 1202, 1203, 1204.0, 1205.0, 1206.0, std::chrono::system_clock::now()});
	orig_track.id = 2001;
	orig_track.cam_index = 20012;
	orig_track.time0 = std::chrono::system_clock::now();
	orig_track.x0_hat = 2002.2;
	orig_track.y0_hat = 2003.3;
	orig_track.z0_hat = 2003.35;
	orig_track.Vx_hat = 2004.4;
	orig_track.Vy_hat = 2005.5;
	orig_track.Vz_hat = 2005.55;
	orig_track.unconfermed_frames_count = 2006;
	orig_track.updated = true;
	orig_track.speed = 2006.67;
	orig_track.std_dev_x = 2007.7;
	orig_track.std_dev_y = 2008.0;
	orig_track.std_dev_xyz = 2009.9;

	CMemStore ms;
	ms.write_native<uint16_t>(12343);
	serialize_dp2_res(ms, orig_track);

	CMemStore rest_ms(ms.data(), ms.size());
	uint16_t msg_code;
	rest_ms.read_native(msg_code);
	EXPECT_EQ(msg_code, 12343);

	Trajectory rest_data;
	bool rest_res = deserialize_dp2_res(rest_ms, rest_data);
	EXPECT_TRUE(rest_res);
	EXPECT_EQ(rest_data.measurements.size(), 3);
	for(int i = 0; i < 3; ++i) {
		EXPECT_EQ(rest_data.measurements[i].id_obj, orig_track.measurements[i].id_obj);
		EXPECT_EQ(rest_data.measurements[i].x, orig_track.measurements[i].x);
		EXPECT_EQ(rest_data.measurements[i].y, orig_track.measurements[i].y);
		EXPECT_EQ(rest_data.measurements[i].iframe, orig_track.measurements[i].iframe);
		EXPECT_EQ(rest_data.measurements[i].Xp, orig_track.measurements[i].Xp);
		EXPECT_EQ(rest_data.measurements[i].Yp, orig_track.measurements[i].Yp);
		EXPECT_EQ(rest_data.measurements[i].Zp, orig_track.measurements[i].Zp);
		EXPECT_EQ(rest_data.measurements[i].time, orig_track.measurements[i].time);
	}
	EXPECT_EQ(rest_data.id, orig_track.id);
	EXPECT_EQ(rest_data.cam_index, orig_track.cam_index);
	EXPECT_EQ(rest_data.time0, orig_track.time0);
	EXPECT_EQ(rest_data.x0_hat, orig_track.x0_hat);
	EXPECT_EQ(rest_data.y0_hat, orig_track.y0_hat);
	EXPECT_EQ(rest_data.z0_hat, orig_track.z0_hat);
	EXPECT_EQ(rest_data.Vx_hat, orig_track.Vx_hat);
	EXPECT_EQ(rest_data.Vy_hat, orig_track.Vy_hat);
	EXPECT_EQ(rest_data.Vz_hat, orig_track.Vz_hat);
	EXPECT_EQ(rest_data.unconfermed_frames_count, orig_track.unconfermed_frames_count);
	EXPECT_EQ(rest_data.updated, orig_track.updated);
	EXPECT_EQ(rest_data.speed, orig_track.speed);
	EXPECT_EQ(rest_data.std_dev_x, orig_track.std_dev_x);
	EXPECT_EQ(rest_data.std_dev_y, orig_track.std_dev_y);
	EXPECT_EQ(rest_data.std_dev_xyz, orig_track.std_dev_xyz);
}
