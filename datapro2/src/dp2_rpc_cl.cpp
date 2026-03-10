#include "dp2_rpc_cl.h"
#include "datapro2.h"
#include "dp2_srobe_mth_chg.h"
#include "rpc_msg_defines.h"
#include "mem_store.h"
#include "datapro1/src/dp1_rpc_data_mrsh.h"
#include "dp2_tr_to_turret.h"
#include <log4cxx/logger.h>
#include "datapro2MultiVision.h"
#include "datetime.h"

using namespace std::chrono;
static auto logger = log4cxx::Logger::getLogger("rpc-sink");
std::unique_ptr<StrobeMethod> measure_processor;
static std::map<int, TDataCalibrationCamera> all_cam_cfg;
static std::map<int, std::deque<TDataRes>> all_cam_mes;
bool start = true;

void dp2_init_measure_proc_algo(const dp2strobe_mth_cfg &config, const binocular_cfg &binocular) {
    measure_processor = std::make_unique<StrobeMethod>(config.min_speed,
                                                       config.max_speed,
                                                       config.max_ptt_frames,
                                                       config.max_tracking_frames,
                                                       config.trajectory_drop_max_confirmed_frames,
                                                       config.trajectory_drop_min_confirmed_frames,
                                                       config.trajectory_drop_min_speed,
                                                       config.trajectory_drop_max_speed,
                                                       config.trajectory_drop_min_acceleration,
                                                       config.trajectory_drop_max_acceleration,
                                                       config.trajectory_drop_max_x_std_dev,
                                                       config.trajectory_drop_max_y_std_dev,
                                                       config.trajectory_drop_max_z_std_dev,
                                                       config.trajectory_drop_max_xyz_std_dev,
                                                       config.trajectory_show_max_confirmed_frames,
                                                       config.trajectory_show_min_confirmed_frames,
                                                       config.trajectory_show_min_speed,
                                                       config.trajectory_show_max_speed,
                                                       config.trajectory_show_min_acceleration,
                                                       config.trajectory_show_max_acceleration,
                                                       config.trajectory_show_max_x_std_dev,
                                                       config.trajectory_show_max_y_std_dev,
                                                       config.trajectory_show_max_z_std_dev,
                                                       config.trajectory_show_max_xyz_std_dev,
                                                       config.trajectory_show_min_frames,
                                                       config.noisy_region_bounding_box_filter_size,
                                                       config.noisy_region_bounding_box_max_objects_limit,
                                                       config.dp2_out_folder,
                                                       config.create_text_file_results,
                                                       config.create_binary_file_results,
                                                       config.create_json_file_results,
                                                       config.linear_model,
                                                       config.k_std,
                                                       config.std_measurement,
                                                       binocular.enabled,
                                                       binocular.use_corners,
                                                       binocular.test_point,
                                                       binocular.match_distance,
                                                       binocular.min_size_queue_frame,
                                                       binocular.max_size_queue_frame,
                                                       binocular.x0test,
                                                       binocular.y0test,
                                                       binocular.Rtest,
                                                       binocular.period_sec,
                                                       config.k_dist);
}

static void on_new_dp1_meas(CMemStore &ms, time_point<steady_clock> rc_rime) {
    TDataRes dp1_data;
    std::vector<Measurement> measurements;
    deserialize_dp1_res(ms, dp1_data);
    LOG4CXX_DEBUG(logger, "rc new meas. cam " << dp1_data.data_cam.cam_index << ", size: " << measurements.size());

    all_cam_mes[dp1_data.data_cam.cam_index].push_back(dp1_data);
    TDataCalibrationCamera &cam_cfg = all_cam_cfg[dp1_data.data_cam.cam_index];
    std::vector<TDataRes> meas_match;
    double match_distance = measure_processor->get_match_distance();
    size_t max_number_vec = measure_processor->get_max_size_queue(),
           min_number_vec = measure_processor->get_min_size_queue();

    std::vector<double> X, Y, Z;
    std::vector<int> num_cams = get_number_cameras(all_cam_cfg);
    double time_index1, time_index2, F = 1.0 / measure_processor->get_period(),
    x0test = measure_processor->get_x0test(), y0test = measure_processor->get_y0test(),
    Rtest = measure_processor->get_Rtest();
    std::vector<cv::Point2f> xy1, xy2;
    auto binocular = measure_processor->get_enabled_binocular();
    auto use_corners = measure_processor->get_use_corners();
    auto use_test_point = measure_processor->get_test_point();
    DateTime t0;
    std::chrono::duration<double> dtc1{}, dtc2{};

    if (!binocular) {
        measurements = measure_processor->cast_dp1_output_to_measurements(dp1_data, cam_cfg);
        measure_processor->process_frame(dp1_data.data_cam, dp1_data.data_frame,
                                                   measurements, cam_cfg);
    } else {
        if (all_cam_mes[num_cams[0]].size() > min_number_vec && all_cam_mes[num_cams[1]].size() > min_number_vec) {
            if (start){
                if (clear_shift_index_frame(all_cam_mes[num_cams[0]],all_cam_mes[num_cams[1]])){
                    if (all_cam_mes[num_cams[0]].front().data_frame.exposureStart>
                            all_cam_mes[num_cams[1]].front().data_frame.exposureStart){
                        t0 = all_cam_mes[num_cams[1]].front().data_frame.exposureStart;
                    } else{
                        t0 = all_cam_mes[num_cams[0]].front().data_frame.exposureStart;
                    }
                    start = false;
                }
            }
            else {
                LOG4CXX_DEBUG(logger, "size_q1: " << all_cam_mes[num_cams[0]].size() << ", size_q2: " << all_cam_mes[num_cams[1]].size());
                LOG4CXX_DEBUG(logger, "numFr1: "
                                        << all_cam_mes[num_cams[0]].front().data_frame.index_frame
                                        << ", numFr2: "
                                        << all_cam_mes[num_cams[1]].front().data_frame.index_frame);
                LOG4CXX_DEBUG(logger, "timeFr1: "
                                        << timePointToString(all_cam_mes[num_cams[0]].front().data_frame.exposureStart, "%H:%M:%S.")
                                        <<", timeFr2: "
                                        << timePointToString(all_cam_mes[num_cams[1]].front().data_frame.exposureStart, "%H:%M:%S."));
                if (use_corners) {
                    TDataRes corners_cam1, corners_cam2;
                    convert_corners_to_meas(
                            all_cam_mes[num_cams[0]].front(),
                            all_cam_mes[num_cams[1]].front(), corners_cam1,
                            corners_cam2);

//                    LOG4CXX_DEBUG(logger, "x_im1: " << xy1[0].x << ", y_im1: " << xy1[0].y);
//                    LOG4CXX_DEBUG(logger, "x_im2: " << xy2[0].x << ", y_im2: " << xy2[0].y);
                    meas_match = selection_meas_to_object_two_cameras(
                            corners_cam1, // TODO
                            corners_cam2, // TODO
                            match_distance);
                } else {
                    if (use_test_point){
                        dtc1 = all_cam_mes[num_cams[0]].front().data_frame.exposureStart - t0;
                        dtc2 = all_cam_mes[num_cams[1]].front().data_frame.exposureStart - t0;

                        time_index1 = 2 * M_PI * dtc1.count() * F;
                        time_index2 = 2 * M_PI * dtc2.count() * F;
                        float x_w1 = x0test + Rtest * cos(time_index1),
                                y_w1 = y0test + Rtest * sin(time_index1),
                                x_w2 = x0test + Rtest * cos(time_index2),
                                y_w2 = y0test + Rtest * sin(time_index2);
                        cv::Point3f xyz_w1(x_w1, y_w1, 0.0);
                        cv::Point3f xyz_w2(x_w2, y_w2, 0.0);
                        std::vector<cv::Point3f> xyz1;
                        std::vector<cv::Point3f> xyz2;
                        xyz1.push_back(xyz_w1);
                        xyz2.push_back(xyz_w2);
                        cv::projectPoints(xyz1,
                                          all_cam_mes[num_cams[0]].front().calib_frame.r_vec,
                                          all_cam_mes[num_cams[0]].front().calib_frame.t_vec,
                                          all_cam_cfg[num_cams[0]].cameraMatrix, all_cam_cfg[num_cams[0]].distCoeffs,
                                          xy1);
                        cv::projectPoints(xyz2,
                                          all_cam_mes[num_cams[1]].front().calib_frame.r_vec,
                                          all_cam_mes[num_cams[1]].front().calib_frame.t_vec,
                                          all_cam_cfg[num_cams[1]].cameraMatrix, all_cam_cfg[num_cams[1]].distCoeffs,
                                          xy2);
                        LOG4CXX_DEBUG(logger, "x_w1: " << xyz1[0].x << ", y_w1: " << xyz1[0].y << ", z_w1: " << xyz1[0].z);
                        LOG4CXX_DEBUG(logger, "x_w2: " << xyz2[0].x << ", y_w2: " << xyz2[0].y << ", z_w2: " << xyz2[0].z);
                        xyz1.clear();xyz2.clear();
                        xyz1.shrink_to_fit();xyz2.shrink_to_fit();
//                        index_point_test++;
                        LOG4CXX_DEBUG(logger, "x_im1: " << xy1[0].x << ", y_im1: " << xy1[0].y);
                        LOG4CXX_DEBUG(logger, "x_im2: " << xy2[0].x << ", y_im2: " << xy2[0].y);
                        TOptionsMeasurement meas_1{TOptionsMeasurement{300, 50,
                                                                       xy1[0].x, xy1[0].y,
                                                                       xy1[0].x, xy1[0].y}},
                                meas_2{TOptionsMeasurement{350, 50,
                                                           xy2[0].x, xy2[0].y,
                                                           xy2[0].x, xy2[0].y}};
                        all_cam_mes[num_cams[0]].front().meas.push_back(meas_1);
                        all_cam_mes[num_cams[1]].front().meas.push_back(meas_2);
                    }
                    meas_match = selection_meas_to_object_two_cameras(
                            all_cam_mes[num_cams[0]].front(), // TODO
                            all_cam_mes[num_cams[1]].front(), // TODO
                            match_distance);
                }
                if (!meas_match.empty() && !meas_match[0].meas.empty() && !meas_match[1].meas.empty()) {
                    LOG4CXX_DEBUG(logger, "match no empty");
                    LOG4CXX_DEBUG(logger, "Size match: " << meas_match.size());
                    LOG4CXX_DEBUG(logger, "Size measurements match1: " << meas_match[0].meas.size());
                    LOG4CXX_DEBUG(logger, "Size measurements match2: " << meas_match[1].meas.size());
                    triangulatePoints(all_cam_cfg[num_cams[0]],
                                      all_cam_cfg[num_cams[1]], meas_match[0],
                                      meas_match[1], X, Y, Z);
                    LOG4CXX_DEBUG(logger, "3D coor: X = " << X.back() << ", Y = " << Y.back() << ", Z = " << Z.back());
                    measurements = measure_processor->cast_dp1_output_to_measurements(meas_match,
                                                                                      all_cam_cfg[num_cams[0]],
                                                                                      all_cam_cfg[num_cams[1]],
                                                                                      X, Y, Z);
                    measure_processor->process_frame_binocular(meas_match[0].data_cam, meas_match[0].data_frame,
                                                               measurements, all_cam_cfg[num_cams[0]]);
                }
                for (int num_cam: num_cams) {
                    all_cam_mes[num_cam].pop_front();
                }
            }
        }
    }

    unsigned int spent_proc_time =
            duration_cast<milliseconds>(steady_clock::now() - rc_rime).count();
    unsigned int dp1_and_dp2_proc_time =
            spent_proc_time + dp1_data.data_frame.dp1_spent_time;
    LOG4CXX_DEBUG(logger, "proc time(ms), dp2: " << spent_proc_time
                                                 << ", total: "
                                                 << dp1_and_dp2_proc_time);
    tr_updates_to_turret(measure_processor.get(), dp1_and_dp2_proc_time);

    if (all_cam_mes[num_cams[0]].size()>max_number_vec || all_cam_mes[num_cams[1]].size()>max_number_vec){
        all_cam_mes[num_cams[0]].clear();
        all_cam_mes[num_cams[1]].clear();
        start = true;
    }
}

std::unique_ptr<rpc_sink> dp2_rpc_cl::cr_sink() {
    return std::make_unique<dp2_rpc_cl>();
}

static void on_camera_cfg(CMemStore &ms) {
    int cam_index;
    ms.read_native(cam_index);
    LOG4CXX_DEBUG(logger, "rc camera config for cam.index : " << cam_index);

    TDataCalibrationCamera &cam_cfg = all_cam_cfg[cam_index];
    deserialize_camera_calibration_data(ms, cam_cfg);
}

void dp2_rpc_cl::on_rd_msg_complite(const uint8_t *data, std::size_t len) {
    time_point<steady_clock> rc_rime = steady_clock::now();
    LOG4CXX_DEBUG(logger, "rc new msg, len: " << len);
    CMemStore ms(data, len);
    uint16_t msg_type;
    ms.read_native(msg_type);
    switch (msg_type) {
        case dp1_to_dp2_rpc_msg_new_measure:
            on_new_dp1_meas(ms, rc_rime);
            break;
        case dp1_to_dp2_camera_calibration_data:
            on_camera_cfg(ms);
            break;
        default:
            LOG4CXX_ERROR(logger, "undefined msg. type: " << msg_type);
    }
}

void dp2_rpc_cl::on_heart_beat() {
    LOG4CXX_DEBUG(logger, "heart - beat");
}

std::vector<Trajectory> get_presented_trajectories() {
    return measure_processor->get_presented_trajectories();
}
