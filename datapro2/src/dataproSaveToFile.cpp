#include <fstream>
#include "dataproSaveToFile.h"

#include <filesystem>
#include <iomanip>
#include <jansson.h>
#include <chrono>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "coordinateTransformation.h"
#include "datetime.h"

std::string indx_to_string(const int &index, const int &width) {
    std::ostringstream ss;
    ss << std::setw(width) << std::setfill('0') << index;
    return ss.str();
}

template<typename T>
void write_to_binary(std::ofstream &stream, const T &value) {
    stream.write(reinterpret_cast<const char *>(&value), sizeof(T));
}

void write_to_binary(std::ofstream &stream, const std::chrono::time_point<std::chrono::system_clock> &value) {
    long long time = value.time_since_epoch().count();
    write_to_binary(stream, time);
}

void write_measurement_to_binary(std::ofstream &stream, const Measurement &m) {
    write_to_binary(stream, m.id_obj);
    write_to_binary(stream, m.x);
    write_to_binary(stream, m.y);
    write_to_binary(stream, m.iframe);
    write_to_binary(stream, m.Xp);
    write_to_binary(stream, m.Yp);
    write_to_binary(stream, m.Zp);
    write_to_binary(stream, m.time);
    write_to_binary(stream, m.Az);
    write_to_binary(stream, m.El);
}

void write_trajectory_to_binary(std::ofstream &stream, const Trajectory &traj, const bool& enabled_binocular) {
    double Az, El, VAz, VEl, aAz, aEl;
    if(!enabled_binocular)
        extrapolationAzEl_V_a(traj, traj.time0, Az, El, VAz, VEl, aAz, aEl);
    else{
//        extrapolation_Az_El_Vaz_Vel_binocular(traj, traj.time0,Az, El,VAz, VEl);
        extrapolationAzEl_4_binocular(traj, traj.time0, Az, El);
        extrapolationVazVel_4_binocular(traj, traj.time0, VAz, VEl);
        aAz = 0; aEl = 0;
    }
    write_to_binary(stream, traj.id);
    write_to_binary(stream, traj.cam_index);
    write_to_binary(stream, traj.time0);
    write_to_binary(stream, traj.x0_hat);
    write_to_binary(stream, traj.y0_hat);
    write_to_binary(stream, traj.z0_hat);
    write_to_binary(stream, traj.Vx_hat);
    write_to_binary(stream, traj.Vy_hat);
    write_to_binary(stream, traj.Vz_hat);
    write_to_binary(stream, traj.ax_hat);
    write_to_binary(stream, traj.ay_hat);
    write_to_binary(stream, traj.az_hat);
    write_to_binary(stream, traj.unconfermed_frames_count);
    write_to_binary(stream, traj.updated);
    write_to_binary(stream, traj.speed);
    write_to_binary(stream, traj.acceleration);
    write_to_binary(stream, traj.std_dev_x);
    write_to_binary(stream, traj.std_dev_y);
    write_to_binary(stream, traj.std_dev_z);
    write_to_binary(stream, traj.std_dev_xyz);

    write_to_binary(stream, traj.strobe_trj);

    write_to_binary(stream, Az);
    write_to_binary(stream, El);
    write_to_binary(stream, VAz);
    write_to_binary(stream, VEl);
    write_to_binary(stream, aAz);
    write_to_binary(stream, aEl);

    unsigned long size = traj.measurements.size();
    write_to_binary(stream, size);
    for (const auto &measurement : traj.measurements) {
        write_measurement_to_binary(stream, measurement);
    }
}

void save_trajectories_as_blob(const std::string &path, const TDataCam &data_cam, const TDataFrame &data_frame,
                               const double& focal_length_x, const double& focal_length_y,
                               const double& optical_centers_x, const double& optical_centers_y, const double& k_sigma,
                               const std::vector<Trajectory> &trajectories, const bool& enabled_binocular) {
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }
    std::string file_name = "C" + indx_to_string(data_cam.cam_index, 3) +
                            "_F" + indx_to_string(data_frame.index_frame, 6) +
                            ".blob";
    std::ofstream file_bin(path + "/" + file_name, std::ios::binary|std::ios::out);

    if (!file_bin.is_open()) {
        std::cerr << "Unable to open file for writing: " << file_name << std::endl;
        return;
    }
    write_to_binary(file_bin, data_cam.cam_index);
    write_to_binary(file_bin, data_cam.Xcam);
    write_to_binary(file_bin, data_cam.Ycam);
    write_to_binary(file_bin, data_cam.Zcam);
    write_to_binary(file_bin, focal_length_x);
    write_to_binary(file_bin, focal_length_y);
    write_to_binary(file_bin, optical_centers_x);
    write_to_binary(file_bin, optical_centers_y);
    write_to_binary(file_bin, data_frame.index_frame);
    write_to_binary(file_bin, k_sigma);

    write_to_binary(file_bin, trajectories.size());
//    for (const auto &traj : trajectories) {
    for (size_t i = 0; i < trajectories.size(); i++) {
        write_trajectory_to_binary(file_bin, trajectories[i],enabled_binocular);
    }
    file_bin.close();
}

void save_trajectories_as_plain_text(const std::string &path, const TDataCam &data_cam, const TDataFrame &data_frame,
                                     const std::vector<Trajectory> &trajectories, const bool& enabled_binocular) {
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }
    std::string file_name = "C" + indx_to_string(data_cam.cam_index, 3) +
                            "_F" + indx_to_string(data_frame.index_frame, 6) +
                            ".txt";

    std::string out_file = path + "/" + file_name;
    std::ofstream outfile(out_file);
    if (!outfile.is_open()) {
        std::cerr << "Unable to open file for writing: " << out_file << std::endl;
        return;
    }
    double Az, El, VAz, VEl, aAz, aEl;
    for (const auto &traj : trajectories) {
        outfile << "Trajectory id: " << traj.id << ", length: " << traj.measurements.size() << "\n";
        for (const auto &m : traj.measurements) {
            outfile << m.iframe << ", " << m.id_obj << ", " << timePointToString(m.time, "%Z %Y-%m-%d %H:%M:%S.") << ", " << m.x << ", " << m.y
                    << ", " << m.Xp << ", " << m.Yp << ", " << m.Zp << ", " << m.Az << ", " << m.El << "\n";
        }
        outfile << "x0_hat: " << traj.x0_hat << ", y0_hat: " << traj.y0_hat << ", z0_hat: " << traj.z0_hat
                << ", Vx_hat: " << traj.Vx_hat << ", Vy_hat: " << traj.Vy_hat << ", Vz_hat: " << traj.Vz_hat
                << ", ax_hat: " << traj.ax_hat << ", ay_hat: " << traj.ay_hat << ", az_hat: " << traj.az_hat
                << ", v: " << traj.speed << ", a: " << traj.acceleration << "\n";
        outfile << "std_dev_x: " << traj.std_dev_x << ", std_dev_y: " << traj.std_dev_y << ", std_dev_z: " << traj.std_dev_z
                << ", std_dev_xyz: " << traj.std_dev_xyz << "\n";
        if(!enabled_binocular)
            extrapolationAzEl_V_a(traj, traj.time0, Az, El, VAz, VEl, aAz, aEl);
        else{
//            extrapolation_Az_El_Vaz_Vel_binocular(traj, traj.time0,Az, El,VAz, VEl);
            extrapolationAzEl_4_binocular(traj, traj.time0, Az, El);
            extrapolationVazVel_4_binocular(traj, traj.time0, VAz, VEl);
            aAz = 0; aEl = 0;
        }
        outfile << "time: " << timePointToString(data_frame.exposureStart, "%Z %Y-%m-%d %H:%M:%S.")
        << ", Az: " << Az << ", El: " << El
        << ", VAz: " << VAz << ", VEl: " << VEl
        << ", aAz: " << aAz << ", aEl: " << aEl <<"\n\n";
    }
    outfile.close();
}

json_t *measurement_to_json(const Measurement &m) {
    json_t *json_measurement = json_object();
    json_object_set_new(json_measurement, "id_obj", json_integer(m.id_obj));
    json_object_set_new(json_measurement, "x", json_real(m.x));
    json_object_set_new(json_measurement, "y", json_real(m.y));
    json_object_set_new(json_measurement, "iframe", json_integer(m.iframe));
    json_object_set_new(json_measurement, "Xp", json_real(m.Xp));
    json_object_set_new(json_measurement, "Yp", json_real(m.Yp));
    json_object_set_new(json_measurement, "Zp", json_real(m.Zp));
    json_object_set_new(json_measurement, "Az", json_real(m.Az));
    json_object_set_new(json_measurement, "El", json_real(m.El));

    long long time = m.time.time_since_epoch().count();
    json_object_set_new(json_measurement, "time", json_integer(time));

    return json_measurement;
}

json_t *trajectory_to_json(const Trajectory &traj) {
    json_t *json_traj = json_object();

    json_object_set_new(json_traj, "id", json_integer(traj.id));
    json_object_set_new(json_traj, "cam_index", json_integer(traj.cam_index));

    long long time0 = traj.time0.time_since_epoch().count();
    json_object_set_new(json_traj, "time0", json_integer(time0));

    json_object_set_new(json_traj, "x0_hat", json_real(traj.x0_hat));
    json_object_set_new(json_traj, "y0_hat", json_real(traj.y0_hat));
    json_object_set_new(json_traj, "z0_hat", json_real(traj.z0_hat));
    json_object_set_new(json_traj, "Vx_hat", json_real(traj.Vx_hat));
    json_object_set_new(json_traj, "Vy_hat", json_real(traj.Vy_hat));
    json_object_set_new(json_traj, "Vz_hat", json_real(traj.Vz_hat));
    json_object_set_new(json_traj, "ax_hat", json_real(traj.ax_hat));
    json_object_set_new(json_traj, "ay_hat", json_real(traj.ay_hat));
    json_object_set_new(json_traj, "az_hat", json_real(traj.az_hat));
    json_object_set_new(json_traj, "unconfirmed_frames_count", json_integer(traj.unconfermed_frames_count));
    json_object_set_new(json_traj, "updated", json_boolean(traj.updated));
    json_object_set_new(json_traj, "speed", json_real(traj.speed));
    json_object_set_new(json_traj, "acceleration", json_real(traj.acceleration));
    json_object_set_new(json_traj, "std_dev_x", json_real(traj.std_dev_x));
    json_object_set_new(json_traj, "std_dev_y", json_real(traj.std_dev_y));
    json_object_set_new(json_traj, "std_dev_z", json_real(traj.std_dev_z));
    json_object_set_new(json_traj, "std_dev_xyz", json_real(traj.std_dev_xyz));

    json_t *json_measurements = json_array();
    for (const auto &m : traj.measurements) {
        json_array_append_new(json_measurements, measurement_to_json(m));
    }
    json_object_set_new(json_traj, "measurements", json_measurements);

    return json_traj;
}

void save_trajectories_as_json(const std::string &path, const TDataCam &data_cam, const TDataFrame &data_frame,
                               const double& focal_length_x, const double& focal_length_y,
                               const double& optical_centers_x, const double& optical_centers_y,
                               const std::vector<Trajectory> &trajectories) {
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }
    std::string file_name = "C" + indx_to_string(data_cam.cam_index, 3) +
                            "_F" + indx_to_string(data_frame.index_frame, 6) +
                            ".json";

    std::string out_file = path + "/" + file_name;

    json_t *json_root = json_object();
    json_object_set_new(json_root, "cam_index", json_integer(data_cam.cam_index));
    json_object_set_new(json_root, "frame_index", json_integer(data_frame.index_frame));
    json_object_set_new(json_root, "focal_length_x", json_real(focal_length_x));
    json_object_set_new(json_root, "focal_length_y", json_real(focal_length_y));
    json_object_set_new(json_root, "optical_centers_x", json_real(optical_centers_x));
    json_object_set_new(json_root, "optical_centers_y", json_real(optical_centers_y));

    long long exposure_start_time = data_frame.exposureStart.time_since_epoch().count();
    json_object_set_new(json_root, "exposure_start", json_integer(exposure_start_time));

    json_t *json_trajectories = json_array();
    for (const auto &traj : trajectories) {
        json_array_append_new(json_trajectories, trajectory_to_json(traj));
    }

    json_object_set_new(json_root, "trajectories", json_trajectories);

    if (json_dump_file(json_root, out_file.c_str(), JSON_INDENT(4)) != 0) {
        std::cerr << "Unable to save JSON to file: " << out_file << std::endl;
    }

    json_decref(json_root);
}
