#ifndef DATAPRO2_H
#define DATAPRO2_H
#include <map>
#include <vector>

#include "datapro2Types.h"

using Duration = std::chrono::duration<double>;

double distance(const Measurement &m1, const Measurement &m2);
bool is_in_strobe(const Measurement &center, const Measurement &point,
                  const double& min_radius, const double& max_radius);//, const bool& coordinate_system_2D

bool object_inside_bounding_box(double x, double y, double box_x1, double box_y1, double box_x2, double box_y2);

class StrobeMethod {
public:
    StrobeMethod(double min_speed, double max_speed, int max_ptt_frames, int max_tracking_frames,
                 int trajectory_drop_max_confirmed_frames, int trajectory_drop_min_confirmed_frames,
                 double trajectory_drop_min_speed, double trajectory_drop_max_speed,
                 double trajectory_drop_min_acceleration, double trajectory_drop_max_acceleration,
                 double trajectory_drop_max_x_std_dev, double trajectory_drop_max_y_std_dev,
                 double trajectory_drop_max_z_std_dev,
                 double trajectory_drop_max_xyz_std_dev,
                 int trajectory_show_max_confirmed_frames, int trajectory_show_min_confirmed_frames,
                 double trajectory_show_min_speed, double trajectory_show_max_speed,
                 double trajectory_show_min_acceleration, double trajectory_show_max_acceleration,
                 double trajectory_show_max_x_std_dev, double trajectory_show_max_y_std_dev,
                 double trajectory_show_max_z_std_dev,
                 double trajectory_show_max_xyz_std_dev,
                 int trajectory_show_min_frames,
                 int noisy_region_bounding_box_filter_size,
                 int noisy_region_bounding_box_max_objects_limit, std::string out_folder,
                 bool create_text_file_results, bool create_binary_file_results,
                 bool create_json_file_results, bool linear_model, double k_std, double std_measurement,
                 bool enabled_binocular, bool use_corners, bool test_point, double match_distance, int min_size_queue_frame,
                 int max_size_queue_frame, double x0test, double y0test, double Rtest, double period_sec, double k_dist);

    std::vector<Trajectory> process_frame(TDataCam &data_cam, TDataFrame &data_frame,
                                               std::vector<Measurement> &measurements, const TDataCalibrationCamera &);

    std::vector<Trajectory> process_frame_binocular(TDataCam &data_cam,
                                                    TDataFrame &data_frame,
                                                    std::vector<Measurement> &measurements,
                                                    const TDataCalibrationCamera &cam1_cfg);

	std::vector<Measurement> filter_noisy_regions(const TDataFrame &data_frame, const TDataCam &data_cam,
												  const std::vector<Measurement> &measurements);

    [[nodiscard]] std::vector<Trajectory> get_presented_trajectories() const;
	[[nodiscard]] std::vector<Trajectory> get_presented_trajectories(int cam_index) const;

	[[nodiscard]] bool is_track_new(unsigned long track_to_check) const {
		return std::find(new_tracks_id.begin(), new_tracks_id.end(), track_to_check) !=  new_tracks_id.end();
	}
	[[nodiscard]] bool is_track_updated(unsigned long track_to_check) const {
		return std::find(updated_tracks_id.begin(), updated_tracks_id.end(), track_to_check) !=  updated_tracks_id.end();
	}
	[[nodiscard]] const std::vector<unsigned long> &get_dropped_track_ids() const { return dropped_tracks_id;}
	void clear_new_updated_dropped_tracks(){
		new_tracks_id.clear();
		updated_tracks_id.clear();
		dropped_tracks_id.clear();
	}

	std::vector<Measurement> cast_dp1_output_to_measurements(TDataRes &dp1_result, const TDataCalibrationCamera &cam_cfg) const;
    std::vector<Measurement> cast_dp1_output_to_measurements(const std::vector<TDataRes> &meas_match,
                                                             const TDataCalibrationCamera& cam1_cfg,
                                                             const TDataCalibrationCamera& cam2_cfg,
                                                             const std::vector<double>& X,
                                                             const std::vector<double>& Y,
                                                             const std::vector<double>& Z) const;
	size_t get_tr_number() const;
	size_t get_ppt_number() const;

    bool get_enabled_binocular() const{return enabled_binocular;};
    bool get_use_corners() const{return use_corners;};
    bool get_test_point() const{return test_point;};
    double get_match_distance() const{return match_distance;};
    int get_min_size_queue() const{return min_size_queue_frame;};
    int get_max_size_queue() const{return max_size_queue_frame;};
    double get_x0test() const{return x0test;};
    double get_y0test() const{return y0test;};
    double get_Rtest() const{return Rtest;};
    double get_period() const{return period_sec;};

private:
	std::map<int, std::vector<PTPoint>> ptts;
	std::map<int, std::vector<Trajectory>> trajectories;

    double min_speed;
    double max_speed;
    unsigned long trajectories_counter = 0;
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
    int noisy_region_noisy_region_bounding_box_max_objects_limit;

    std::string out_folder;

    bool create_text_file_results;
    bool create_binary_file_results;
	bool create_json_file_results;

    bool linear_model;
    double k_std;
    double std_measurement;

    double k_dist;

    bool enabled_binocular;
    bool use_corners;
    double match_distance;
    int min_size_queue_frame, max_size_queue_frame;
    bool test_point;
    double x0test, y0test, Rtest;
    double period_sec;

	std::vector<unsigned long> new_tracks_id;
	std::vector<unsigned long> updated_tracks_id;
	std::vector<unsigned long> dropped_tracks_id;

    void calculate_statistical_data(std::vector<Trajectory> &trajectories) const;

	void resolve_tracks_mutation(std::vector<Trajectory> &trajectories);

    static void update_trajectory_parameters(Trajectory &trajectory);

    static void update_trajectory_parameters_qm(Trajectory &trajectory);

    [[nodiscard]] bool trajectory_matches_requirements(const Trajectory &trajectory, const bool& drop) const ;

    void drop_trajectories(std::vector<Trajectory> &trajectories);

	static void calc_parameters_linear_model(double sum_x, double sum_x_dt,
											 double sum_dt, double sum_dt2,
											 int N, double denominator,
											 double &x0_hat, double &Vx_hat);

	static void calc_parameters_quadratic_model(double sum_x, double sum_x_dt,
												double sum_dt, double sum_dt2,
												double sum_x_dt2, double sum_dt3,
												double sum_dt4,
												int N, double denominator,
												double &x0_hat, double &Vx_hat, double &ax_hat);
	bool try_merge_trajectories(Trajectory &tr_to, Trajectory &tr_from) const;
};

#endif //DATAPRO2_H
