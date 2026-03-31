#include <jansson.h>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <limits.h>
#include <unistd.h>
#include "dp2_cfg.h"
#include "m_json_unique_ptr.h"
#include "m_json_cfg_reader.h"

using std::string;
using namespace dp2;

namespace {
	std::string trim_copy(std::string value) {
		const auto begin = value.find_first_not_of(" \t\r\n");
		if (begin == std::string::npos)
			return {};
		const auto end = value.find_last_not_of(" \t\r\n");
		return value.substr(begin, end - begin + 1);
	}

	std::filesystem::path get_executable_path() {
		char buffer[PATH_MAX]{};
		const auto length = ::readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
		if (length <= 0)
			return std::filesystem::current_path();
		buffer[length] = '\0';
		return std::filesystem::path(buffer);
	}

	std::filesystem::path find_repo_root() {
		auto current = get_executable_path().parent_path();
		while (!current.empty()) {
			if (std::filesystem::exists(current / "AGENTS.md") &&
				std::filesystem::exists(current / "CMakeLists.txt")) {
				return current;
			}
			if (current == current.root_path())
				break;
			current = current.parent_path();
		}
		return std::filesystem::current_path();
	}

	bool try_read_resources_root(const std::filesystem::path &cfg_file, std::filesystem::path &result, const std::filesystem::path &repo_root) {
		std::ifstream input(cfg_file);
		if (!input)
			return false;

		std::string line;
		while (std::getline(input, line)) {
			line = trim_copy(line);
			if (line.empty() || line[0] == '#')
				continue;

			const auto sep_pos = line.find('=');
			if (sep_pos == std::string::npos)
				continue;

			const auto key = trim_copy(line.substr(0, sep_pos));
			const auto value = trim_copy(line.substr(sep_pos + 1));
			if (key != "resources_root" || value.empty())
				continue;

			std::filesystem::path candidate(value);
			if (candidate.is_relative())
				candidate = repo_root / candidate;
			result = candidate.lexically_normal();
			return true;
		}

		return false;
	}

	std::filesystem::path get_resources_root() {
		if (const char *env_root = std::getenv("AMANITA_RESOURCES_DIR"); env_root && *env_root) {
			return std::filesystem::path(env_root);
		}

		const auto repo_root = find_repo_root();
		std::filesystem::path from_cfg;
		if (try_read_resources_root(repo_root / "amanita_resources.local.conf", from_cfg, repo_root))
			return from_cfg;
		if (try_read_resources_root(repo_root / "amanita_resources.conf", from_cfg, repo_root))
			return from_cfg;

		return (repo_root / "AmanitaResources").lexically_normal();
	}

	std::string resolve_resource_path(const std::string &raw_path) {
		static const std::string token{"${AMANITA_RESOURCES_DIR}"};
		if (raw_path.compare(0, token.size(), token) != 0)
			return raw_path;

		auto suffix = raw_path.substr(token.size());
		while (!suffix.empty() && suffix.front() == '/')
			suffix.erase(suffix.begin());

		return (get_resources_root() / suffix).lexically_normal().string();
	}
}

dp2_cfg::dp2_cfg(const char *cfg_fname){
	auto json_root = json_unique_ptr_create(json_load_file(cfg_fname, 0, nullptr));
	if (!json_root)
		throw std::logic_error(string{"error on opening config file: "} + cfg_fname);

	{
		jansson_cfg_obj_reader root_reader(json_root.get());
		dp2_id = root_reader.read_int_param("id");
	}

	jansson_cfg_obj_reader cfg_reader(json_root.get(), "server");
	port = cfg_reader.read_int_param("port");

	jansson_cfg_obj_reader strobe_mth_reader(json_root.get(), "strobe-method-par");
	strobe_mth.min_speed = strobe_mth_reader.read_double_param("min_speed");
	strobe_mth.max_speed = strobe_mth_reader.read_double_param("max_speed");
	strobe_mth.max_ptt_frames = strobe_mth_reader.read_int_param("max_ptt_frames");
	strobe_mth.max_tracking_frames = strobe_mth_reader.read_int_param("max_tracking_frames");
	strobe_mth.trajectory_drop_max_confirmed_frames = strobe_mth_reader.read_int_param("trajectory_drop_max_confirmed_frames");
	strobe_mth.trajectory_drop_min_confirmed_frames = strobe_mth_reader.read_int_param("trajectory_drop_min_confirmed_frames");
	strobe_mth.trajectory_drop_min_speed = strobe_mth_reader.read_double_param("trajectory_drop_min_speed");
	strobe_mth.trajectory_drop_max_speed = strobe_mth_reader.read_double_param("trajectory_drop_max_speed");
	strobe_mth.trajectory_drop_min_acceleration = strobe_mth_reader.read_double_param("trajectory_drop_min_acceleration");
	strobe_mth.trajectory_drop_max_acceleration = strobe_mth_reader.read_double_param("trajectory_drop_max_acceleration");
	strobe_mth.trajectory_drop_max_x_std_dev = strobe_mth_reader.read_double_param("trajectory_drop_max_x_std_dev");
	strobe_mth.trajectory_drop_max_y_std_dev = strobe_mth_reader.read_double_param("trajectory_drop_max_y_std_dev");
	strobe_mth.trajectory_drop_max_z_std_dev = strobe_mth_reader.read_double_param("trajectory_drop_max_z_std_dev");
	strobe_mth.trajectory_drop_max_xyz_std_dev = strobe_mth_reader.read_double_param("trajectory_drop_max_xyz_std_dev");
	strobe_mth.trajectory_show_max_confirmed_frames = strobe_mth_reader.read_int_param("trajectory_show_max_confirmed_frames");
	strobe_mth.trajectory_show_min_confirmed_frames = strobe_mth_reader.read_int_param("trajectory_show_min_confirmed_frames");
	strobe_mth.trajectory_show_min_speed = strobe_mth_reader.read_double_param("trajectory_show_min_speed");
	strobe_mth.trajectory_show_max_speed = strobe_mth_reader.read_double_param("trajectory_show_max_speed");
	strobe_mth.trajectory_show_min_acceleration = strobe_mth_reader.read_double_param("trajectory_show_min_acceleration");
	strobe_mth.trajectory_show_max_acceleration = strobe_mth_reader.read_double_param("trajectory_show_max_acceleration");
	strobe_mth.trajectory_show_max_x_std_dev = strobe_mth_reader.read_double_param("trajectory_show_max_x_std_dev");
	strobe_mth.trajectory_show_max_y_std_dev = strobe_mth_reader.read_double_param("trajectory_show_max_y_std_dev");
	strobe_mth.trajectory_show_max_z_std_dev = strobe_mth_reader.read_double_param("trajectory_show_max_z_std_dev");
	strobe_mth.trajectory_show_max_xyz_std_dev = strobe_mth_reader.read_double_param("trajectory_show_max_xyz_std_dev");
	strobe_mth.trajectory_show_min_frames = strobe_mth_reader.read_int_param("trajectory_show_min_frames");
	strobe_mth.noisy_region_bounding_box_filter_size = strobe_mth_reader.read_int_param("noisy_region_bounding_box_filter_size");
	strobe_mth.noisy_region_bounding_box_max_objects_limit = strobe_mth_reader.read_int_param("noisy_region_bounding_box_max_objects_limit");
	strobe_mth.dp1_out_folder = resolve_resource_path(strobe_mth_reader.read_string_param("dp1_out_folder"));
	strobe_mth.dp2_out_folder = resolve_resource_path(strobe_mth_reader.read_string_param("dp2_out_folder"));
	strobe_mth.create_text_file_results = strobe_mth_reader.read_bool_param("create_text_file_results");
	strobe_mth.create_binary_file_results = strobe_mth_reader.read_bool_param("create_binary_file_results");
	strobe_mth.create_json_file_results = strobe_mth_reader.read_bool_param("create_json_file_results");
    strobe_mth.linear_model = strobe_mth_reader.read_bool_param("linear_model");
    strobe_mth.k_std = strobe_mth_reader.read_double_param("k_std");
    strobe_mth.std_measurement = strobe_mth_reader.read_double_param("std_measurement");
    strobe_mth.k_dist = strobe_mth_reader.read_double_param("k_dist");
	{
		jansson_cfg_obj_reader turret_exch_cfg_reader(json_root.get(), "turret_exch");
		turret_exch.enabled = turret_exch_cfg_reader.read_bool_param("enabled");
		turret_exch.host = turret_exch_cfg_reader.read_string_param("host");
		turret_exch.port = turret_exch_cfg_reader.read_int_param("port");
		turret_exch.tr_interval_ms = turret_exch_cfg_reader.read_int_param("tr_interval_ms");
		if(turret_exch.tr_interval_ms < 100)
			turret_exch.tr_interval_ms = 300;
		turret_exch.reconn_interval_s = turret_exch_cfg_reader.read_int_param("reconnect_interval_s");
		if(turret_exch.reconn_interval_s < 1)
			turret_exch.reconn_interval_s = 3;
	}

    {
        jansson_cfg_obj_reader binocular_reader(json_root.get(), "binocular");
        binocular.enabled = binocular_reader.read_bool_param("enabled");
        binocular.use_corners = binocular_reader.read_bool_param("use_corners");
        binocular.test_point = binocular_reader.read_bool_param("test_point");
        binocular.min_size_queue_frame = binocular_reader.read_int_param("min_size_queue_frame");
        binocular.max_size_queue_frame = binocular_reader.read_int_param("max_size_queue_frame");
        binocular.match_distance = binocular_reader.read_double_param("match_distance");
        binocular.x0test = binocular_reader.read_double_param("x0test");
        binocular.y0test = binocular_reader.read_double_param("y0test");
        binocular.Rtest = binocular_reader.read_double_param("Rtest");
        binocular.period_sec = binocular_reader.read_double_param("period_sec");
    }
}
