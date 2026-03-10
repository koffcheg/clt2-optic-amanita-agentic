#include "datapro2.h"
#include <numeric>
#include <limits>
#include <filesystem>
#include "log4cxx/logger.h"
#include "coordinateTransformation.h"
#include "dataproSaveToFile.h"
#include "calculationTrajectory.h"

static auto logger = log4cxx::Logger::getLogger("dp2-data-marsh");
bool current_frame = false;

bool is_in_strobe_trajectory(const Measurement& point, const TStrobe& strobe) {
    bool strobe_x = strobe.x_min <= point.Xp && strobe.x_max >= point.Xp,
            strobe_y = strobe.y_min <= point.Yp && strobe.y_max >= point.Yp,
            strobe_z = strobe.z_min <= point.Zp && strobe.z_max >= point.Zp;
    return strobe_x && strobe_y && strobe_z;
}

bool object_inside_bounding_box(double x, double y, double box_x1, double box_y1, double box_x2, double box_y2) {
    return (x >= box_x1 && x < box_x2) && (y >= box_y1 && y < box_y2);
}

std::vector<Measurement> StrobeMethod::filter_noisy_regions(const TDataFrame &data_frame, const TDataCam &data_cam,
                                                            const std::vector<Measurement> &measurements) {
    std::vector<Measurement> filtered_measurements;

    for (int i = 0; i < data_frame.width; i += noisy_region_bounding_box_filter_size) {
        for (int j = 0; j < data_frame.height; j += noisy_region_bounding_box_filter_size) {
            std::vector<Measurement> box_objects;

            for (const auto &element: measurements) {
                if (
                    object_inside_bounding_box(
                        element.x, element.y,
                        i, j,
                        i + noisy_region_bounding_box_filter_size,
                        j + noisy_region_bounding_box_filter_size
                    )
                ) {
                    box_objects.push_back(element);
                }
            }

            if (box_objects.size() > noisy_region_noisy_region_bounding_box_max_objects_limit) {
                continue;
            }

            for (const auto &obj: box_objects) {
                filtered_measurements.push_back(obj);
            }
        }
    }
    return filtered_measurements;
}

StrobeMethod::StrobeMethod(double min_speed, double max_speed, int max_ptt_frames, int max_tracking_frames,
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
                           int max_size_queue_frame,
                           double x0test, double y0test, double Rtest, double period_sec, double k_dist)
    : min_speed(min_speed), max_speed(max_speed), max_ptt_frames(max_ptt_frames),
      max_tracking_frames(max_tracking_frames),
      trajectory_drop_max_confirmed_frames(trajectory_drop_max_confirmed_frames),
      trajectory_drop_min_confirmed_frames(trajectory_drop_min_confirmed_frames),
      trajectory_drop_min_speed(trajectory_drop_min_speed),
      trajectory_drop_max_speed(trajectory_drop_max_speed),
      trajectory_drop_min_acceleration(trajectory_drop_min_acceleration),
      trajectory_drop_max_acceleration(trajectory_drop_max_acceleration),
      trajectory_drop_max_x_std_dev(trajectory_drop_max_x_std_dev),
      trajectory_drop_max_y_std_dev(trajectory_drop_max_y_std_dev),
      trajectory_drop_max_z_std_dev(trajectory_drop_max_z_std_dev),
      trajectory_drop_max_xyz_std_dev(trajectory_drop_max_xyz_std_dev),
      trajectory_show_max_confirmed_frames(trajectory_show_max_confirmed_frames),
      trajectory_show_min_confirmed_frames(trajectory_show_min_confirmed_frames),
      trajectory_show_min_speed(trajectory_show_min_speed),
      trajectory_show_max_speed(trajectory_show_max_speed),
      trajectory_show_min_acceleration(trajectory_show_min_acceleration),
      trajectory_show_max_acceleration(trajectory_show_max_acceleration),
      trajectory_show_max_x_std_dev(trajectory_show_max_x_std_dev),
      trajectory_show_max_y_std_dev(trajectory_show_max_y_std_dev),
      trajectory_show_max_z_std_dev(trajectory_show_max_z_std_dev),
      trajectory_show_max_xyz_std_dev(trajectory_show_max_xyz_std_dev),
      trajectory_show_min_frames(trajectory_show_min_frames),
      noisy_region_bounding_box_filter_size(noisy_region_bounding_box_filter_size),
      noisy_region_noisy_region_bounding_box_max_objects_limit(noisy_region_bounding_box_max_objects_limit),
      out_folder(std::move(out_folder)),
      create_text_file_results(create_text_file_results),
      create_binary_file_results(create_binary_file_results),
      create_json_file_results(create_json_file_results),
      linear_model(linear_model),
      k_std(k_std),
      std_measurement(std_measurement),
      enabled_binocular(enabled_binocular),
      use_corners(use_corners),
      match_distance(match_distance),
      min_size_queue_frame(min_size_queue_frame),
      max_size_queue_frame(max_size_queue_frame),
      test_point(test_point),
      x0test(x0test),
      y0test(y0test),
      Rtest(Rtest),
      period_sec(period_sec),
      k_dist(k_dist){
}

std::vector<Trajectory> StrobeMethod::process_frame(TDataCam &data_cam,
                                                         TDataFrame &data_frame,
                                                         std::vector<Measurement> &measurements,
														 const TDataCalibrationCamera &cam_cfg) {
    std::vector<PTPoint> &cam_ptts = ptts[data_cam.cam_index];
    std::vector<Trajectory> &cam_trajectories = trajectories[data_cam.cam_index];
    double dist, dispersion_xp, dispersion_yp, dispersion_zp;
    bool in_strobe;
    DateTime t_curr_frame = data_frame.exposureStart;
//    TStrobe strobe{0,0,0,0,0,0,0,0,0,0};

    if(data_frame.index_frame==13){
        current_frame= true;
    }
    measurements = filter_noisy_regions(data_frame, data_cam, measurements);
    std::vector assigned_measurements(measurements.size(), false);
    clear_new_updated_dropped_tracks();
    calc_dispersion_measurement_3D(std_measurement, std_measurement,
                                   data_frame.Az, data_frame.El,
                                   cam_cfg.cameraMatrix.at<double>(0,0), cam_cfg.cameraMatrix.at<double>(1,1),
                                   dispersion_xp, dispersion_yp,dispersion_zp);

    // Check existing trajectories
    Measurement extrapolated;
    for (auto &trajectory: cam_trajectories) {
        if(current_frame){
            if(trajectory.id == 0){
                int nnnn = 0;
            }
        }
        trajectory.updated = false;
        if (linear_model || (trajectory.measurements.size()==2)){
            extrapolated = extrapolate_position(trajectory,t_curr_frame);
            trajectory.strobe_trj = calc_strobe_linear(trajectory, extrapolated, k_std, dispersion_xp, dispersion_yp, dispersion_zp);
        }
        else {
            extrapolated = extrapolate_position_qm(trajectory,t_curr_frame);
            trajectory.strobe_trj = calc_strobe_quadratic(trajectory, extrapolated, k_std, dispersion_xp, dispersion_yp, dispersion_zp);
        }

        Measurement *best_match = nullptr;
        double best_distance = std::numeric_limits<double>::max();
        int best_match_index = -1;


        for (size_t i = 0; i < measurements.size(); ++i) {
            if(current_frame & trajectory.id == 0){
                if(measurements[i].id_obj == 0){
                    int nnnn = 0;
                }
            }
            in_strobe =is_in_strobe_trajectory(measurements[i], trajectory.strobe_trj);

            if (!assigned_measurements[i] && in_strobe) {

                dist = distance(extrapolated, measurements[i]);

                if (dist < best_distance) {

                    best_distance = dist;
                    best_match = &measurements[i];
                    best_match_index = (int)i;
                }
            }
        }

        if (best_match) {
            if (trajectory.measurements.size() == max_tracking_frames) {
                trajectory.measurements.pop_front();
            }
            trajectory.measurements.push_back(*best_match);

            if (linear_model)
                update_trajectory_parameters(trajectory);
            else
                update_trajectory_parameters_qm(trajectory);

            if (trajectory.unconfermed_frames_count > 0) {
                --trajectory.unconfermed_frames_count;
            }
            assigned_measurements[best_match_index] = true;
            trajectory.updated = true;
        } else {
            trajectory.unconfermed_frames_count++;
        }
    }

    // Check initial trajectory points (PTTs)
    std::vector<PTPoint> remaining_ptts;
    for (auto &ptt: cam_ptts) {
        Measurement *best_match = nullptr;
        double best_distance = std::numeric_limits<double>::max(), dt;
        int best_match_index = -1;
        DateTime time_start = ptt.measurement.time, t_stop;

        for (size_t i = 0; i < measurements.size(); ++i) {
            t_stop = measurements[i].time;
            dt = diff_time_in_seconds(time_start,t_stop);
            if (!assigned_measurements[i] && is_in_strobe(ptt.measurement, measurements[i],
                                                          min_speed * dt, max_speed * dt)) {//, coordinate_system_2D

                dist = distance(ptt.measurement, measurements[i]);

                if (dist < best_distance) {
                    best_distance = dist;
                    best_match = &measurements[i];
                    best_match_index = (int)i;
                }
            }
        }

        if (best_match) {
            Trajectory new_trajectory = {
                {ptt.measurement, *best_match},
                trajectories_counter++,
                data_cam.cam_index
            };
            // TODO
            update_trajectory_parameters(new_trajectory);

			//check if it can be merged
			bool was_merged = false;
			for (auto &trajectory: cam_trajectories) {
				if (try_merge_trajectories(trajectory, new_trajectory)){
					was_merged = true;
					break;
				}
			}
			if(!was_merged) {
				new_trajectory.updated = true;
				cam_trajectories.push_back(new_trajectory);
			}

			assigned_measurements[best_match_index] = true;
        } else {
            ptt.passed_frames += 1;
            if (ptt.passed_frames < max_ptt_frames) {
                remaining_ptts.push_back(ptt);
            }
        }
    }
    cam_ptts = std::move(remaining_ptts);

    // Remaining measurements become new PTTs
    for (size_t i = 0; i < measurements.size(); ++i) {
        if (!assigned_measurements[i]) {
            PTPoint new_ptt = {measurements[i]};
            cam_ptts.push_back(new_ptt);
        }
    }

    calculate_statistical_data(cam_trajectories);
    drop_trajectories(cam_trajectories);
    resolve_tracks_mutation(cam_trajectories);

    std::vector<Trajectory> trajectories_to_show = get_presented_trajectories(data_cam.cam_index);

    if (create_text_file_results) {
        save_trajectories_as_plain_text(out_folder, data_cam, data_frame, trajectories_to_show, enabled_binocular);
    }
    if (create_binary_file_results) {
        save_trajectories_as_blob(out_folder, data_cam, data_frame,
                                  cam_cfg.cameraMatrix.at<double>(0,0), cam_cfg.cameraMatrix.at<double>(1,1),
                                  cam_cfg.cameraMatrix.at<double>(0,2), cam_cfg.cameraMatrix.at<double>(1,2), k_std,
                                  trajectories_to_show, enabled_binocular);
    }
    if (create_json_file_results) {
        save_trajectories_as_json(out_folder, data_cam, data_frame,
                                  cam_cfg.cameraMatrix.at<double>(0,0), cam_cfg.cameraMatrix.at<double>(1,1),
                                  cam_cfg.cameraMatrix.at<double>(0,2), cam_cfg.cameraMatrix.at<double>(1,2),
                                  trajectories_to_show);
    }
    return trajectories_to_show;
}

std::vector<Trajectory> StrobeMethod::process_frame_binocular(TDataCam &data_cam,
                                                              TDataFrame &data_frame,
                                                              std::vector<Measurement> &measurements,
                                                              const TDataCalibrationCamera &cam1_cfg) {
    std::vector<PTPoint> &cam_ptts = ptts[data_cam.cam_index];
    std::vector<Trajectory> &cam_trajectories = trajectories[data_cam.cam_index];
    double dist;
    bool in_strobe;
    DateTime t_curr_frame = data_frame.exposureStart;

    if(data_frame.index_frame==5000){
        current_frame= true;
    }
    measurements = filter_noisy_regions(data_frame, data_cam, measurements);
    std::vector assigned_measurements(measurements.size(), false);
    clear_new_updated_dropped_tracks();

    // Check existing trajectories
    Measurement extrapolated;
    for (auto &trajectory: cam_trajectories) {
        if(current_frame){
            if(trajectory.id == 0){
                int nnnn = 0;
            }
        }
        trajectory.updated = false;
        bool check_model_first;
        if (linear_model || (trajectory.measurements.size()==2)){
            check_model_first = true;
        }
        else {
            check_model_first = false;
        }

        Measurement *best_match = nullptr;
        double best_distance = std::numeric_limits<double>::max();
        int best_match_index = -1;


        for (size_t i = 0; i < measurements.size(); ++i) {
            if(current_frame & trajectory.id == 0){
                if(measurements[i].id_obj == 0){
                    int nnnn = 0;
                }
            }
            if (check_model_first){
                extrapolated = extrapolate_position(trajectory,t_curr_frame);
                trajectory.strobe_trj = calc_strobe_linear(trajectory, extrapolated, k_std,
                                                           measurements[i].dispersionX,
                                                           measurements[i].dispersionY,
                                                           measurements[i].dispersionZ);
            }
            else{
                extrapolated = extrapolate_position_qm(trajectory,t_curr_frame);
                trajectory.strobe_trj = calc_strobe_quadratic(trajectory, extrapolated, k_std,
                                                              measurements[i].dispersionX,
                                                              measurements[i].dispersionY,
                                                              measurements[i].dispersionZ);
            }

            in_strobe =is_in_strobe_trajectory(measurements[i], trajectory.strobe_trj);

            if (!assigned_measurements[i] && in_strobe) {

                dist = distance(extrapolated, measurements[i]);

                if (dist < best_distance) {

                    best_distance = dist;
                    best_match = &measurements[i];
                    best_match_index = (int)i;
                }
            }
        }

        if (best_match) {
            if (trajectory.measurements.size() == max_tracking_frames) {
                trajectory.measurements.pop_front();
            }
            trajectory.measurements.push_back(*best_match);

            if (linear_model)
                update_trajectory_parameters(trajectory);
            else
                update_trajectory_parameters_qm(trajectory);

            if (trajectory.unconfermed_frames_count > 0) {
                --trajectory.unconfermed_frames_count;
            }
            assigned_measurements[best_match_index] = true;
            trajectory.updated = true;
        } else {
            trajectory.unconfermed_frames_count++;
        }
    }

    // Check initial trajectory points (PTTs)
    std::vector<PTPoint> remaining_ptts;
    for (auto &ptt: cam_ptts) {
        Measurement *best_match = nullptr;
        double best_distance = std::numeric_limits<double>::max(), dt;
        int best_match_index = -1;
        DateTime time_start = ptt.measurement.time, t_stop;

        for (size_t i = 0; i < measurements.size(); ++i) {
            t_stop = measurements[i].time;
            dt = diff_time_in_seconds(time_start,t_stop);
            if (!assigned_measurements[i] && is_in_strobe(ptt.measurement, measurements[i],
                                                          min_speed * dt, max_speed * dt)) {//, coordinate_system_2D

                dist = distance(ptt.measurement, measurements[i]);

                if (dist < best_distance) {
                    best_distance = dist;
                    best_match = &measurements[i];
                    best_match_index = (int)i;
                }
            }
        }

        if (best_match) {
            Trajectory new_trajectory = {
                    {ptt.measurement, *best_match},
                    trajectories_counter++,
                    data_cam.cam_index
            };
            // TODO
            update_trajectory_parameters(new_trajectory);

            //check if it can be merged
            bool was_merged = false;
            for (auto &trajectory: cam_trajectories) {
                if (try_merge_trajectories(trajectory, new_trajectory)){
                    was_merged = true;
                    break;
                }
            }
            if(!was_merged) {
                new_trajectory.updated = true;
                cam_trajectories.push_back(new_trajectory);
            }

            assigned_measurements[best_match_index] = true;
        } else {
            ptt.passed_frames += 1;
            if (ptt.passed_frames < max_ptt_frames) {
                remaining_ptts.push_back(ptt);
            }
        }
    }
    cam_ptts = std::move(remaining_ptts);

    // Remaining measurements become new PTTs
    for (size_t i = 0; i < measurements.size(); ++i) {
        if (!assigned_measurements[i]) {
            PTPoint new_ptt = {measurements[i]};
            cam_ptts.push_back(new_ptt);
        }
    }

    calculate_statistical_data(cam_trajectories);
    drop_trajectories(cam_trajectories);
    resolve_tracks_mutation(cam_trajectories);

    std::vector<Trajectory> trajectories_to_show = get_presented_trajectories(data_cam.cam_index);

    if (create_text_file_results) {
        save_trajectories_as_plain_text(out_folder, data_cam, data_frame, trajectories_to_show, enabled_binocular);
    }
    if (create_binary_file_results) {
        save_trajectories_as_blob(out_folder, data_cam, data_frame,
                                  cam1_cfg.cameraMatrix.at<double>(0,0), cam1_cfg.cameraMatrix.at<double>(1,1),
                                  cam1_cfg.cameraMatrix.at<double>(0,2), cam1_cfg.cameraMatrix.at<double>(1,2), k_std,
                                  trajectories_to_show, enabled_binocular);
    }
    if (create_json_file_results) {
        save_trajectories_as_json(out_folder, data_cam, data_frame,
                                  cam1_cfg.cameraMatrix.at<double>(0,0), cam1_cfg.cameraMatrix.at<double>(1,1),
                                  cam1_cfg.cameraMatrix.at<double>(0,2), cam1_cfg.cameraMatrix.at<double>(1,2),
                                  trajectories_to_show);
    }
    return trajectories_to_show;
}

bool StrobeMethod::trajectory_matches_requirements(const Trajectory &trajectory, const bool& drop) const {
    int max_confirmed_frames = drop ? trajectory_drop_max_confirmed_frames : trajectory_show_max_confirmed_frames;
    int min_confirmed_frames = drop ? trajectory_drop_min_confirmed_frames : trajectory_show_min_confirmed_frames;
    double check_min_speed = drop ? trajectory_drop_min_speed : trajectory_show_min_speed;
    double check_max_speed = drop ? trajectory_drop_max_speed : trajectory_show_max_speed;
    double min_acceleration = drop ? trajectory_drop_min_acceleration : trajectory_show_min_acceleration;
    double max_acceleration = drop ? trajectory_drop_max_acceleration : trajectory_show_max_acceleration;
    double max_x_std_dev = drop ? trajectory_drop_max_x_std_dev : trajectory_show_max_x_std_dev;
    double max_y_std_dev = drop ? trajectory_drop_max_y_std_dev : trajectory_show_max_y_std_dev;
    double max_z_std_dev = drop ? trajectory_drop_max_z_std_dev : trajectory_show_max_z_std_dev;
    double max_xyz_std_dev = drop ? trajectory_drop_max_xyz_std_dev : trajectory_show_max_xyz_std_dev;

    if (max_confirmed_frames - trajectory.unconfermed_frames_count < min_confirmed_frames){
        LOG4CXX_DEBUG(logger, "Drop Traj: MinMaxFrames");
        return false;
    }
    if (trajectory.speed < check_min_speed || trajectory.speed > check_max_speed){
        LOG4CXX_DEBUG(logger, "Drop Traj: speed  = " << trajectory.speed);
        return false;
    }
    if (!linear_model)
        if (trajectory.acceleration < min_acceleration || trajectory.acceleration > max_acceleration){
            LOG4CXX_DEBUG(logger, "Drop Traj:acceleration  = " << trajectory.acceleration);
            return false;
        }
    if (trajectory.std_dev_x > max_x_std_dev){
        LOG4CXX_DEBUG(logger, "Drop Traj: std_dev_x  = " << trajectory.std_dev_x);
        return false;
    }
    if (trajectory.std_dev_y > max_y_std_dev){
        LOG4CXX_DEBUG(logger, "Drop Traj: std_dev_y  = " << trajectory.std_dev_y);
        return false;
    }
    if (trajectory.std_dev_z > max_z_std_dev){
        LOG4CXX_DEBUG(logger, "Drop Traj: std_dev_z  = " << trajectory.std_dev_z);
        return false;
    }
    if (trajectory.std_dev_xyz > max_xyz_std_dev){
        LOG4CXX_DEBUG(logger, "Drop Traj: std_dev_xyz  = " << trajectory.std_dev_xyz);
        return false;
    }
    if (!drop && trajectory.measurements.size() < trajectory_show_min_frames){
        LOG4CXX_DEBUG(logger, "Drop Traj: show_min_frames  = " << trajectory_show_min_frames);
        return false;
    }
    return true;
}

void StrobeMethod::drop_trajectories(std::vector<Trajectory> &updating_trajectories) {
    updating_trajectories.erase(
        std::remove_if(updating_trajectories.begin(), updating_trajectories.end(), [&](const Trajectory &traj) {
            bool drop_trajectory = !trajectory_matches_requirements(traj, true);
            if (drop_trajectory) {
                dropped_tracks_id.push_back(traj.id);
            }
            return drop_trajectory;
        }),
        updating_trajectories.end()
    );
}

std::vector<Trajectory> StrobeMethod::get_presented_trajectories() const {
    std::vector<Trajectory> filtered_trajectories;
    for (const auto &cam_trajs: trajectories) {
        for (size_t i=0; i < cam_trajs.second.size(); i++) {
            if (trajectory_matches_requirements(cam_trajs.second[i], false)) {
                filtered_trajectories.push_back(cam_trajs.second[i]);
            }
        }
    }
    return filtered_trajectories;
}

std::vector<Trajectory> StrobeMethod::get_presented_trajectories(int cam_index) const {
    std::vector<Trajectory> all_trajectories = get_presented_trajectories();
    std::vector<Trajectory> filtered_trajectories;

    for (size_t i=0; i < all_trajectories.size(); i++) {
        if (all_trajectories[i].cam_index == cam_index) {
            filtered_trajectories.push_back(all_trajectories[i]);
        }
    }
    return filtered_trajectories;
}

void StrobeMethod::calc_parameters_linear_model(const double sum_x, const double sum_x_dt,
                                                const double sum_dt, const double sum_dt2,
                                                const int N, const double denominator,
                                                double& x0_hat, double& Vx_hat){
    x0_hat = (sum_dt2 * sum_x - sum_dt * sum_x_dt) / denominator;
    Vx_hat = (N * sum_x_dt - sum_dt * sum_x) / denominator;
}

void StrobeMethod::calc_parameters_quadratic_model(const double sum_x, const double sum_x_dt,
                                                  const double sum_dt, const double sum_dt2,
                                                  const double sum_x_dt2,
                                                  const double sum_dt3, const double sum_dt4,
                                                  const int N, const double denominator,
                                                  double& x0_hat, double& Vx_hat, double& ax_hat){
    x0_hat = (sum_x_dt2 * sum_dt2 * sum_dt2 - sum_x_dt * sum_dt2 * sum_dt3 -
                         sum_dt4 * sum_x * sum_dt2 + sum_x * sum_dt3 * sum_dt3 - sum_dt * sum_x_dt2 * sum_dt3 +
                         sum_dt * sum_dt4 * sum_x_dt) / denominator;
    Vx_hat = (sum_dt2 * sum_dt2 * sum_x_dt - N * sum_dt4 * sum_x_dt +
                         N * sum_dt3 * sum_x_dt2 + sum_dt * sum_dt4 * sum_x - sum_dt2 * sum_dt3 *  sum_x -
                         sum_dt * sum_dt2 * sum_x_dt2) / denominator;
    ax_hat = (sum_x_dt2 * sum_dt * sum_dt - sum_x_dt * sum_dt * sum_dt2 -
                         sum_dt3 * sum_x * sum_dt + sum_x * sum_dt2 * sum_dt2 - N* sum_x_dt2 * sum_dt2 +
                         N * sum_dt3 * sum_x_dt) / denominator;
}

void StrobeMethod::update_trajectory_parameters(Trajectory &trajectory) {
    trajectory.time0 = trajectory.measurements.front().time;
    double sum_x = 0, sum_y = 0, sum_z = 0;
    double sum_x_dt = 0, sum_y_dt = 0, sum_z_dt = 0;
    double sum_dt = 0, sum_dt2 = 0, denominator;
    int N;

    calculation_parameters_linear_least_squares(trajectory,trajectory.time0,
                                                sum_x,sum_y,sum_z,
                                                sum_x_dt, sum_y_dt, sum_z_dt,
                                                sum_dt, sum_dt2,
                                                N, denominator);

    calc_parameters_linear_model(sum_x, sum_x_dt, sum_dt, sum_dt2, N, denominator,
                                 trajectory.x0_hat, trajectory.Vx_hat);
    calc_parameters_linear_model(sum_y, sum_y_dt, sum_dt, sum_dt2, N, denominator,
                                 trajectory.y0_hat, trajectory.Vy_hat);
    calc_parameters_linear_model(sum_z, sum_z_dt, sum_dt, sum_dt2, N, denominator,
                                 trajectory.z0_hat, trajectory.Vz_hat);
    trajectory.ax_hat = 0;
    trajectory.ay_hat = 0;
    trajectory.az_hat = 0;
}

void StrobeMethod::update_trajectory_parameters_qm(Trajectory &trajectory) {
    trajectory.time0 = trajectory.measurements.front().time;
    double sum_x = 0, sum_y = 0, sum_z = 0;
    double sum_x_dt = 0, sum_y_dt = 0, sum_z_dt = 0;
    double sum_x_dt2 = 0, sum_y_dt2 = 0, sum_z_dt2 = 0;
    double sum_dt = 0, sum_dt2 = 0, sum_dt3 = 0, sum_dt4 = 0, denominator;
    int N;

    calculation_parameters_quadratic_least_squares(trajectory,trajectory.time0,
                                                   sum_x, sum_y, sum_z,
                                                   sum_x_dt, sum_y_dt, sum_z_dt,
                                                   sum_x_dt2, sum_y_dt2, sum_z_dt2,
                                                   sum_dt, sum_dt2, sum_dt3, sum_dt4,
                                                   N, denominator);

    calc_parameters_quadratic_model(sum_x, sum_x_dt, sum_dt, sum_dt2, sum_x_dt2,
                                    sum_dt3, sum_dt4, (int)N, denominator,
                                    trajectory.x0_hat, trajectory.Vx_hat, trajectory.ax_hat);

    calc_parameters_quadratic_model(sum_y, sum_y_dt, sum_dt, sum_dt2, sum_y_dt2,
                                    sum_dt3, sum_dt4, (int)N, denominator,
                                    trajectory.y0_hat, trajectory.Vy_hat, trajectory.ay_hat);

    calc_parameters_quadratic_model(sum_z, sum_z_dt, sum_dt, sum_dt2, sum_z_dt2,
                                    sum_dt3, sum_dt4, (int)N, denominator,
                                    trajectory.z0_hat, trajectory.Vz_hat, trajectory.az_hat);
}

void StrobeMethod::calculate_statistical_data(std::vector<Trajectory> &updating_trajectories) const {
    double predicted_x, predicted_y, predicted_z,
            x_diff, y_diff, z_diff,
            sum_x_squared, sum_y_squared, sum_z_squared, t, t2,
            variance_x, variance_y, variance_z;
//    std::chrono::duration<double> DT1;
    for (auto &trajectory: updating_trajectories) {
        sum_x_squared = 0, sum_y_squared = 0, sum_z_squared = 0;
        const size_t n = trajectory.measurements.size();

        if (n == 0) {
            continue;
        }

        for (const auto &measurement: trajectory.measurements) {
            t = diff_time_in_seconds(trajectory.time0, measurement.time );
//            t = (double)(measurement.time - trajectory.time0).count();
            t2 = t * t;

            predicted_x = trajectory.Vx_hat * t + trajectory.x0_hat;
            predicted_y = trajectory.Vy_hat * t + trajectory.y0_hat;
            predicted_z = trajectory.Vz_hat * t + trajectory.z0_hat;

            if (!linear_model){
                predicted_x += trajectory.ax_hat * t2;
                predicted_y += trajectory.ay_hat * t2;
                predicted_z += trajectory.az_hat * t2;
            }

            x_diff = measurement.Xp - predicted_x;
            y_diff = measurement.Yp - predicted_y;
            z_diff = measurement.Zp - predicted_z;


            sum_x_squared += std::pow(x_diff, 2);
            sum_y_squared += std::pow(y_diff, 2);
            sum_z_squared += std::pow(z_diff, 2);
        }

        variance_x = sum_x_squared / (int)n;
        variance_y = sum_y_squared / (int)n;
        variance_z = sum_z_squared / (int)n;

        trajectory.std_dev_x = std::sqrt(variance_x);
        trajectory.std_dev_y = std::sqrt(variance_y);
        trajectory.std_dev_z = std::sqrt(variance_z);
        trajectory.std_dev_xyz = std::sqrt(variance_x + variance_y + variance_z);
        trajectory.speed = std::sqrt(pow(trajectory.Vx_hat, 2) + pow(trajectory.Vy_hat, 2) + pow(trajectory.Vz_hat, 2));
        trajectory.acceleration = std::sqrt(pow(trajectory.ax_hat, 2) + pow(trajectory.ay_hat, 2) + pow(trajectory.az_hat, 2));
        LOG4CXX_DEBUG(logger, "stat: stdX  = " << trajectory.std_dev_x << ", stdY  = " << trajectory.std_dev_y << ", stdZ  = " << trajectory.std_dev_z);
    }
}

void StrobeMethod::resolve_tracks_mutation(std::vector<Trajectory> &updating_trajectories) {
    for (auto &trajectory: updating_trajectories) {
        if (!trajectory.updated)
            continue;

        // If trajectory is updated, it is considered as new if it has only 2 measurements
        if (trajectory.measurements.size() == 2) {
            new_tracks_id.push_back(trajectory.id);
        } else {
            updated_tracks_id.push_back(trajectory.id);
        }
    }
}

std::vector<Measurement> StrobeMethod::cast_dp1_output_to_measurements(TDataRes &dp1_result,
                                                                       const TDataCalibrationCamera &cam_cfg) const {
	std::vector<Measurement> measurements;
	int iframe = dp1_result.data_frame.index_frame;
	DateTime exposureStart = dp1_result.data_frame.exposureStart;
	// TODO
	double Xp, Yp, Zp, Az = 0, El = 0;
	for (const auto &element: dp1_result.meas) {
		xy2XYZ(dp1_result.data_frame, dp1_result.data_cam, element.x_weight, element.y_weight,
               cam_cfg.cameraMatrix.at<double>(0,0), cam_cfg.cameraMatrix.at<double>(1,1),
               cam_cfg.cameraMatrix.at<double>(0,2), cam_cfg.cameraMatrix.at<double>(1,2),
			   Xp, Yp, Zp);
		if (dp1_result.data_frame.turretInfoValid){
			Az = dp1_result.data_frame.Az;
			El = dp1_result.data_frame.El;
		}
		measurements.push_back(Measurement({element.id_obj,element.x_weight, element.y_weight, iframe, Xp, Yp, Zp,
											exposureStart, Az, El}));
	}
	return measurements;
}

std::vector<Measurement> StrobeMethod::cast_dp1_output_to_measurements(const std::vector<TDataRes> &meas_match,
                                                                       const TDataCalibrationCamera& cam1_cfg,
                                                                       const TDataCalibrationCamera& cam2_cfg,
                                                                       const std::vector<double>& X,
                                                                       const std::vector<double>& Y,
                                                                       const std::vector<double>& Z) const {
    std::vector<Measurement> measurements;
    double Az = 0, El = 0, dispersion_xp, dispersion_yp, dispersion_zp;
    int iframe = meas_match[0].data_frame.index_frame;
    DateTime exposureStart = meas_match[0].data_frame.exposureStart;
    if (meas_match[0].data_frame.turretInfoValid){
        Az = meas_match[0].data_frame.Az;
        El = meas_match[0].data_frame.El;
    }
    double fc_1 = .5*(cam1_cfg.cameraMatrix.at<double>(0,0) + cam1_cfg.cameraMatrix.at<double>(1,1)),
            fc_2 = .5*(cam2_cfg.cameraMatrix.at<double>(0,0) + cam2_cfg.cameraMatrix.at<double>(1,1)),
            c1x = cam1_cfg.cameraMatrix.at<double>(0,2),
            c1y = cam1_cfg.cameraMatrix.at<double>(1,2),
            c2x = cam2_cfg.cameraMatrix.at<double>(0,2),
            c2y = cam2_cfg.cameraMatrix.at<double>(1,2);
    double zm = 100;//mm //TODO
    for (size_t i = 0; i < meas_match[0].meas.size(); i++){
        calc_dispersion_measurement_3D_binocular(std_measurement, std_measurement,
                                                 fc_1, fc_2, // mean focal length camera 1 & 2
                                                 c1x, c1y, // center frame camera 1
                                                 c2x, c2y, // center frame camera 2
                                                 meas_match[0].meas[i].x_weight, meas_match[0].meas[i].y_weight, // coordinates of the object in the frame of the first camera
                                                 meas_match[1].meas[i].x_weight, meas_match[1].meas[i].y_weight, // coordinates of the object in the frame of the second camera
                                                 zm, // scale, distance plane of two frame
                                                 meas_match[0].calib_frame.R_matrix, meas_match[0].calib_frame.t_vec,
                                                 meas_match[1].calib_frame.R_matrix, meas_match[1].calib_frame.t_vec,
                                                 dispersion_xp, dispersion_yp, dispersion_zp);
        LOG4CXX_DEBUG(logger, "dispX = " << dispersion_xp << ", dispY = " << dispersion_yp << ", dispZ = " << dispersion_zp);
        measurements.push_back(Measurement({meas_match[0].meas[i].id_obj,
                                            meas_match[0].meas[i].x_weight,
                                            meas_match[0].meas[i].y_weight,
                                            iframe, X[i], Y[i], Z[i],
                                            exposureStart, Az, El,
                                            dispersion_xp, dispersion_yp,dispersion_zp}));
    }
    return measurements;
}

bool StrobeMethod::try_merge_trajectories(Trajectory &tr_to, Trajectory &tr_from) const{
	if(tr_to.updated)
		return false;
    LOG4CXX_DEBUG(logger, "checking merger new traj.id " << tr_from.id << " with traj.id " << tr_to.id);

	double dist_between_tr = distance(tr_from.measurements.front(), tr_to.measurements.back());
	double delta_t = diff_time_in_seconds(tr_to.measurements.back().time, tr_from.measurements.front().time);
	double new_tr_v = sqrt(tr_from.Vx_hat*tr_from.Vx_hat + tr_from.Vy_hat*tr_from.Vy_hat + tr_from.Vz_hat*tr_from.Vz_hat);
	double possible_dist = delta_t*new_tr_v;
	possible_dist *= k_dist;
    LOG4CXX_DEBUG(logger, "possible dist=" << possible_dist << ", dist. between traj.=" << dist_between_tr);
	if(possible_dist < dist_between_tr)
		return false;

	tr_to.updated = true;
	tr_to.measurements = std::move(tr_from.measurements);
	update_trajectory_parameters(tr_to);
	tr_to.unconfermed_frames_count = 0;

	LOG4CXX_DEBUG(logger, "merged new traj. into " << tr_to.id);
	return true;
}

size_t StrobeMethod::get_tr_number() const {
	size_t res{};
	for (const auto &el: trajectories)
		res += el.second.size();
	return res;
}

size_t StrobeMethod::get_ppt_number() const {
	size_t res{};
	for (const auto &el: ptts)
		res += el.second.size();
	return res;
}

