#ifndef CLT_OPTIC_DP2_SROBE_MTH_CHG_H
#define CLT_OPTIC_DP2_SROBE_MTH_CHG_H

#include <string>

struct binocular_cfg {
    bool enabled = true;
    bool use_corners = false;
    double match_distance = 2.0;
    int min_size_queue_frame = 10;
    int max_size_queue_frame = 100;
    bool test_point = true;
    double x0test = 90.0, y0test = 60.0, Rtest = 240.0;
    double period_sec = 60;
};

struct dp2strobe_mth_cfg {
	double min_speed;
	double max_speed;
	int max_ptt_frames;
	int max_tracking_frames;
	int trajectory_drop_max_confirmed_frames;
	int trajectory_drop_min_confirmed_frames;
	double trajectory_drop_min_speed;
	double trajectory_drop_max_speed;
	double trajectory_drop_min_acceleration;
	double trajectory_drop_max_acceleration;
	double trajectory_drop_max_x_std_dev;
	double trajectory_drop_max_y_std_dev;
	double trajectory_drop_max_z_std_dev;
	double trajectory_drop_max_xyz_std_dev;
	int trajectory_show_max_confirmed_frames;
	int trajectory_show_min_confirmed_frames;
	double trajectory_show_min_speed;
	double trajectory_show_max_speed;
	double trajectory_show_min_acceleration;
	double trajectory_show_max_acceleration;
	double trajectory_show_max_x_std_dev;
	double trajectory_show_max_y_std_dev;
	double trajectory_show_max_z_std_dev;
	double trajectory_show_max_xyz_std_dev;
	int trajectory_show_min_frames;
	int noisy_region_bounding_box_filter_size;
	int noisy_region_bounding_box_max_objects_limit;
	std::string dp1_out_folder;
	std::string dp2_out_folder;
	bool create_text_file_results;
	bool create_binary_file_results;
	bool create_json_file_results;
    bool linear_model;
    double k_std;
    double std_measurement;
    double k_dist;
};

#endif //CLT_OPTIC_DP2_SROBE_MTH_CHG_H
