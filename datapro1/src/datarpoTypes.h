#ifndef DATAPRO_TYPE_H
#define DATAPRO_TYPE_H
#include <opencv2/opencv.hpp>

using DateTime = std::chrono::time_point<std::chrono::system_clock>;

struct TDataFrame {
    int index_frame;
	uint32_t dp1_spent_time;	//ms
    DateTime exposureStart;///
    int width;///
    int height;///
    float exposureLength;   // s////
    float focalLength;      // m///
    float pixelWidth;       // m///
    float pixelHeight;      // m///
    float El;///
    float Az;///
    float V_el;///
    float V_az;///
    bool turretInfoValid;
};

struct TDataCalibrationCamera {
    cv::Mat cameraMatrix = cv::Mat_<double>(3,3), distCoeffs = cv::Mat_<double>(1,5);
    double avg_reprojection_error = 1.0;
    float square_size = 1;
    int board_width = 1, board_height = 1;
};

struct TDataCalibrationFrame{
    cv::Mat R_matrix = cv::Mat_<double>(1,1), t_vec = cv::Mat_<double>(1,1),
            r_vec = cv::Mat_<double>(1,1), descriptors_scene = cv::Mat_<double>(1,1);
    std::vector<cv::Point2f> chessboard_corners{cv::Point2f(0,0)};
    std::vector<cv::KeyPoint> keypoints_scene{cv::KeyPoint()};
};

struct TDataCam {
    int cam_index;
    double Xcam = 0, Ycam = 0, Zcam = 0;
};

struct TOptionsMeasurement {
	int id_obj = 0, 
		num_pix_obj = 1;
	double x_weight = 0,
		y_weight = 0,
        x_rec = 0,
        y_rec =  0,
		rec_width = 0,
		rec_height = 0,
		mean_brightness_obj = 0,
		std_brightness_obj = 0,
		mean_brightness_im = 0,
		std_brightness_im = 0,
		obj_angel = 0,
        obj_angel_moment = 0,
        obj_eccentricity = 0;
};

struct TDrawMeasurement {
    cv::RotatedRect box;
    cv::Rect rect;
};

struct TDataRes {
    TDataCam data_cam;
    TDataFrame data_frame;
    std::vector<TOptionsMeasurement> meas;
//    TDataCalibrationCamera calib_cam;
    TDataCalibrationFrame calib_frame;
};

struct TPixelObj {
    cv::Point coor;
    float br;
};

struct TDataproConfig{
    int numFragX, //
    numFragY,
    border_x,
    border_y,
    sizePartY,
    sizePartX,
    sizePartYend,
    sizePartXend;
    bool update_bg_model = false;
};

struct TDataproVar{
    std::vector<TDrawMeasurement> data_draw;
    std::vector<cv::Mat> vec_frag, vec_bgmask;
    std::vector<cv::Ptr<cv::BackgroundSubtractor>> vec_bgsubtractor;
};

struct TFolder{
    std::string data_bin = "data_bin",
            data_pix = "data_pix",
            frame_input = "frame_input",
            frame_background = "frame_background",
            frame_diff = "frame_diff",
            frame_segment = "frame_segment",
            frame_coor = "frame_coor";
};
#endif
