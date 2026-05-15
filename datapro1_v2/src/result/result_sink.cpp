#include "dp1v2/result/result_sink.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <ctime>

#include "dataproSaveToFile.h"
#include "datetime.h"
#include "dp1v2/stages/calibration_contract.hpp"
#include "dp1_tr_res2dp2.h"
#include "m_json_unique_ptr.h"

namespace {

struct ArtifactOutputSettings {
    bool enabled = false;
    bool json_enabled = false;
    bool binocular_enabled = false;
    std::string out_folder = "datapro1_v2_output";
    std::string data_bin_folder = "data_bin";
};

TDataCalibrationCamera g_camera_calibration;
ArtifactOutputSettings g_artifact_config;
bool g_dp2_enabled = false;
bool g_result_sink_initialized = false;

bool write_result_json(const TDataRes &result, const std::filesystem::path &out_path, const std::string &file_name) {
    auto json_root = json_unique_ptr_create(json_object());
    {
        json_t *cam_json = json_object();
        json_object_set_new(cam_json, "cam_index", json_integer(result.data_cam.cam_index));
        json_object_set_new(cam_json, "Xcam", json_real(result.data_cam.Xcam));
        json_object_set_new(cam_json, "Ycam", json_real(result.data_cam.Ycam));
        json_object_set_new(cam_json, "Zcam", json_real(result.data_cam.Zcam));
        json_object_set_new(cam_json, "pixel_width", json_real(result.data_frame.pixelWidth));
        json_object_set_new(cam_json, "pixel_height", json_real(result.data_frame.pixelHeight));
        json_object_set_new(cam_json, "focal_length", json_real(result.data_frame.focalLength));
        json_object_set_new(json_root.get(), "Camera data", cam_json);
    }
    {
        json_t *turret_json = json_object();
        json_object_set_new(turret_json, "Az", json_real(result.data_frame.Az));
        json_object_set_new(turret_json, "El", json_real(result.data_frame.El));
        json_object_set_new(turret_json, "V_az", json_real(result.data_frame.V_az));
        json_object_set_new(turret_json, "V_el", json_real(result.data_frame.V_el));
        json_object_set_new(json_root.get(), "Turret data", turret_json);
    }
    {
        json_t *data_frame_json = json_object();
        json_object_set_new(data_frame_json, "index_frame", json_integer(result.data_frame.index_frame));
        json_object_set_new(data_frame_json, "exposureStart",
                            json_string(timePointToString(result.data_frame.exposureStart, "%Z %Y-%m-%d %H:%M:%S.").c_str()));
        json_object_set_new(data_frame_json, "exposureLength", json_real(result.data_frame.exposureLength));
        json_object_set_new(data_frame_json, "width", json_integer(result.data_frame.width));
        json_object_set_new(data_frame_json, "height", json_integer(result.data_frame.height));
        json_object_set_new(json_root.get(), "Data frame", data_frame_json);
    }
    {
        json_t *measures_json = json_object();
        json_object_set_new(measures_json, "Size", json_integer(static_cast<json_int_t>(result.meas.size())));
        json_t *meas_array_json = json_array();
        for (const auto &measurement : result.meas) {
            json_t *curr_meas_json = json_object();
            json_object_set_new(curr_meas_json, "id_obj", json_integer(measurement.id_obj));
            json_object_set_new(curr_meas_json, "num_pix_obj", json_integer(measurement.num_pix_obj));
            json_object_set_new(curr_meas_json, "x_weight", json_real(measurement.x_weight));
            json_object_set_new(curr_meas_json, "y_weight", json_real(measurement.y_weight));
            json_object_set_new(curr_meas_json, "x_rec", json_real(measurement.x_rec));
            json_object_set_new(curr_meas_json, "y_rec", json_real(measurement.y_rec));
            json_object_set_new(curr_meas_json, "rec_height", json_real(measurement.rec_height));
            json_object_set_new(curr_meas_json, "rec_width", json_real(measurement.rec_width));
            json_object_set_new(curr_meas_json, "obj_angel", json_real(measurement.obj_angel));
            json_object_set_new(curr_meas_json, "obj_angel_moment", json_real(measurement.obj_angel_moment));
            json_object_set_new(curr_meas_json, "obj_eccentricity", json_real(measurement.obj_eccentricity));
            json_object_set_new(curr_meas_json, "mean_brightness_obj", json_real(measurement.mean_brightness_obj));
            json_object_set_new(curr_meas_json, "std_brightness_obj", json_real(measurement.std_brightness_obj));
            json_array_append_new(meas_array_json, curr_meas_json);
        }
        json_object_set_new(measures_json, "Measurements", meas_array_json);
        json_object_set_new(json_root.get(), "Measurements frame", measures_json);
    }
    {
        json_t *frame_calib_json = json_object();
        json_t *r_matrix_array_json = json_array();
        for (int r = 0; r < dp1v2::kRotationMatrixRows; ++r) {
            for (int c = 0; c < dp1v2::kRotationMatrixCols; ++c) {
                json_array_append_new(r_matrix_array_json, json_real(result.calib_frame.R_matrix.at<double>(r, c)));
            }
        }
        json_object_set_new(frame_calib_json, "R matrix", r_matrix_array_json);

        json_t *t_vector_array_json = json_array();
        for (int i = 0; i < dp1v2::kPoseVectorElements; ++i) {
            json_array_append_new(t_vector_array_json, json_real(result.calib_frame.t_vec.at<double>(i)));
        }
        json_object_set_new(frame_calib_json, "t vector", t_vector_array_json);
        json_object_set_new(json_root.get(), "Frame calibration", frame_calib_json);
    }

    const auto json_path = out_path / (file_name + ".json");
    return json_dump_file(json_root.get(), json_path.string().c_str(), JSON_INDENT(4)) == 0;
}

dp1v2::ResultSinkStatus write_result_artifacts(const TDataRes &result) {
    if (!g_artifact_config.enabled) {
        return dp1v2::ResultSinkStatus::ArtifactSkipped;
    }

    const auto out_path = std::filesystem::path(g_artifact_config.out_folder) / g_artifact_config.data_bin_folder;
    std::filesystem::create_directories(out_path);

    std::string file_name;
    creating_file_name(std::time(nullptr), result.data_cam.cam_index, result.data_frame.index_frame, file_name);

    std::ofstream file_bin(out_path / (file_name + ".blob"), std::ios::out | std::ios::binary);
    if (!file_bin.is_open()) {
        return dp1v2::ResultSinkStatus::Failed;
    }

    save_data_cam(file_bin, result.data_cam);
    save_data_frame(file_bin, result.data_frame);
    save_data_meas(file_bin, result.meas);
    const int check_binocular = g_artifact_config.binocular_enabled ? 1 : 0;
    file_bin.write(reinterpret_cast<const char *>(&check_binocular), sizeof(check_binocular));
    if (g_artifact_config.binocular_enabled) {
        save_data_cam_calibration(file_bin, g_camera_calibration);
        save_data_frame_calibration(file_bin, result.calib_frame);
    }
    if (!file_bin.good()) {
        return dp1v2::ResultSinkStatus::Failed;
    }
    file_bin.close();

    if (g_artifact_config.json_enabled && !write_result_json(result, out_path, file_name)) {
        return dp1v2::ResultSinkStatus::Failed;
    }

    return dp1v2::ResultSinkStatus::Accepted;
}

} // namespace

namespace dp1v2 {

const char *result_sink_status_to_cstr(const ResultSinkStatus status) {
    switch (status) {
        case ResultSinkStatus::Accepted:
            return "accepted";
        case ResultSinkStatus::SendSkipped:
            return "send_skipped";
        case ResultSinkStatus::ArtifactSkipped:
            return "artifact_skipped";
        case ResultSinkStatus::Failed:
            return "failed";
        default:
            return "unknown";
    }
}

void initialize_result_sink(const DP2ConnectionConfig &config, const int cam_index, const TDataCalibrationCamera &camera_calibration) {
    g_camera_calibration = camera_calibration;
    g_dp2_enabled = config.enabled && config.mode != DP2ConnectionMode::Disabled;
    if (g_dp2_enabled) {
        ns_datapro1::init_connect_to_dp2(
            config.host,
            config.port,
            config.reconnect_interval_s,
            cam_index,
            g_camera_calibration);
    }
    g_result_sink_initialized = true;
}

ResultSinkOutcome publish_result_to_sinks(const TDataRes &result) {
    if (!g_result_sink_initialized) {
        return ResultSinkOutcome{
            .send_status = ResultSinkStatus::Failed,
            .artifact_status = ResultSinkStatus::ArtifactSkipped,
            .reason = "result_sink_not_initialized",
        };
    }

    ResultSinkStatus send_status = ResultSinkStatus::SendSkipped;
    if (g_dp2_enabled) {
        ns_datapro1::send_res_to_dp2(result);
        send_status = ResultSinkStatus::Accepted;
    }

    const auto artifact_status = write_result_artifacts(result);
    return ResultSinkOutcome{
        .send_status = send_status,
        .artifact_status = artifact_status,
        .reason = artifact_status == ResultSinkStatus::Failed ? "artifact_write_failed" : "result_sinks_invoked",
    };
}

} // namespace dp1v2
