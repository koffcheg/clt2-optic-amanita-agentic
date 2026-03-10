#include "dp1_config.h"
#include <stdexcept>
#include <m_json_unique_ptr.h>
#include <map>
#include "m_json_cfg_reader.h"

using std::string;

namespace ns_datapro1 {
	prg_config::prg_config(const char *cfg_fname, int camera_index) : ipc_cfg{camera_index} {

		auto json_root = json_unique_ptr_create(json_load_file(cfg_fname, 0, nullptr));
		if (!json_root)
			throw std::logic_error(string{"error on opening config file: "} + cfg_fname);

		json_t *json_cfg = json_object_get(json_root.get(), "config");
		if (!json_cfg)
			throw std::logic_error("error on config file, there is no root-node 'config'");

        load_cfg_binocular(json_cfg);
		load_cfg_segment(json_cfg);
		load_cfg_filters(json_cfg);
        load_cfg_median_bg(json_cfg);
        load_cfg_subtractor(json_cfg);
        load_cfg_source(json_cfg);
		load_dp2_conn_cfg(json_cfg);
        load_cfg_test(json_cfg);
		load_cfg_calc_limits(json_cfg);
		read_cfg_multiproc(json_cfg);

		jansson_cfg_obj_reader cfg_reader(json_root.get(), "config");
		def_border = cfg_reader.read_int_param("def_border");
	}

	void prg_config::read_cfg_multiproc(json_t *json_upper){
		jansson_cfg_obj_reader cfg_reader(json_upper, "multiprocess");

		multiproc.tiles_factor = cfg_reader.read_int_param("tiles_factor");
		if(multiproc.tiles_factor > 48)
			throw std::logic_error("error on config file, too big 'tiles_factor': " + std::to_string(multiproc.tiles_factor));

		multiproc.ocv_num_thread = cfg_reader.read_int_param("ocv_num_thread");
		if(multiproc.ocv_num_thread < 0 || multiproc.ocv_num_thread > 24)
			throw std::logic_error("error on config file, too small or too big 'ocv_num_thread': " + std::to_string(multiproc.ocv_num_thread));

		multiproc.dp1_num_thread = cfg_reader.read_int_param("dp1_num_thread");
		if(multiproc.dp1_num_thread > 48)
			throw std::logic_error("error on config file, too big 'dp1_num_thread': " + std::to_string(multiproc.dp1_num_thread));
	}

    void prg_config::load_cfg_binocular(json_t *json_upper) {
        jansson_cfg_obj_reader cfg_reader(json_upper, "binocular");

        binocular.switched = cfg_reader.read_bool_param("switched");
        binocular.file_camera_settings = cfg_reader.read_string_param("file_camera_settings");
        binocular.minHessian = cfg_reader.read_int_param("minHessian");
        binocular.using_template = cfg_reader.read_bool_param("using_template");

        jansson_cfg_obj_reader cam_reader(json_object_get(json_upper, "binocular"), "camera");

        /* ---- R_matrix ---- */
        const json_t *r_arr = cam_reader.read_array("R_matrix");      // очікуємо ТІЛЬКИ масив
        if (!json_is_array(r_arr) || json_array_size(r_arr) != 9)
            throw std::logic_error("'camera.R_matrix' must be an array of 9 numbers");

        binocular.R_matrix.clear();
        size_t idx; json_t *val;
        json_array_foreach(r_arr, idx, val) {
            if (!json_is_number(val))
                throw std::logic_error("non-numeric value in 'camera.R_matrix'");
            binocular.R_matrix.push_back(json_number_value(val));
        }

        /* ---- t_vector ---- */
        const json_t *t_arr = cam_reader.read_array("t_vector");      // очікуємо ТІЛЬКИ масив
        if (!json_is_array(t_arr) || json_array_size(t_arr) != 3)
            throw std::logic_error("'camera.t_vector' must be an array of 3 numbers");

        binocular.t_vector.clear();
        json_array_foreach(t_arr, idx, val) {
            if (!json_is_number(val))
                throw std::logic_error("non-numeric value in 'camera.t_vector'");
            binocular.t_vector.push_back(json_number_value(val));
        }
    }

	void prg_config::load_cfg_segment(json_t *json_upper) {
		jansson_cfg_obj_reader cfg_reader(json_upper, "segment");

		segment.element_str_w = cfg_reader.read_int_param("element_str_w");
		segment.element_str_h = cfg_reader.read_int_param("element_str_h");
		segment.number_iter = cfg_reader.read_int_param("number_iter");
		segment.element_str_type = cfg_reader.read_int_param("element_str_type");
		segment.level = cfg_reader.read_int_param("level");
		segment.level_max = cfg_reader.read_int_param("level_max");
		segment.threshold_type = cfg_reader.read_int_param("threshold_type");
		segment.contour_retrieval = cfg_reader.read_int_param("contour_retrieval");
		segment.contour_approximation = cfg_reader.read_int_param("contour_approximation");
        segment.min_segment_size = cfg_reader.read_int_param("min_segment_size");
        segment.max_segment_size = cfg_reader.read_int_param("max_segment_size");
        segment.max_height_to_width_ratio = cfg_reader.read_real_param("max_height_to_width_ratio");
	}

	void prg_config::load_cfg_filters(json_t *json_upper) {
		json_t *filters_cfg = json_object_get(json_upper, "filter");
		if (!filters_cfg)
			throw std::logic_error("error on config file, there is no root-node 'filter'");
		load_single_filter(filters_cfg, "corr", filters.corr);
		load_single_filter(filters_cfg, "matched", filters.matched);
		load_single_filter(filters_cfg, "blur", filters.blur);
        load_median_filter(filters_cfg, "median", filters.median);
	}

	void prg_config::load_single_filter(json_t *filters, const char *filter_name, cfg_one_filter &res_filter) {
		jansson_cfg_obj_reader cfg_reader(filters, filter_name);
		res_filter.width = cfg_reader.read_int_param("width");
		res_filter.height = cfg_reader.read_int_param("height");
		res_filter.switched = cfg_reader.read_bool_param("switched");
	}

    void prg_config::load_median_filter(json_t *filters, const char *filter_name, cfg_median_filter &res_filter) {
        jansson_cfg_obj_reader cfg_reader(filters, filter_name);

        res_filter.window_1_size = cfg_reader.read_int_param("window_1_size");
        res_filter.window_2_size = cfg_reader.read_int_param("window_2_size");
        res_filter.rejection = cfg_reader.read_bool_param("rejection");
        res_filter.switched = cfg_reader.read_bool_param("switched");
    }

    void prg_config::load_cfg_median_bg(json_t *json_upper) {
        jansson_cfg_obj_reader cfg_reader(json_upper, "median_bg");

        median_bg.switched = cfg_reader.read_bool_param("switched");
        median_bg.num_frame = cfg_reader.read_int_param("num_frame");
        median_bg.sampling_period = cfg_reader.read_int_param("sampling_period");
        median_bg.affine_transfor = cfg_reader.read_bool_param("affine_transfor");
    }

    void prg_config::load_cfg_subtractor(json_t *json_upper) {
        jansson_cfg_obj_reader cfg_reader(json_upper, "subtractor");

        subtractor.subtractorKNN = cfg_reader.read_bool_param("subtractorKNN");
        subtractor.detectShadows = cfg_reader.read_bool_param("detectShadows");
        subtractor.historyKNN = cfg_reader.read_int_param("historyKNN");
        subtractor.historyMOG2 = cfg_reader.read_int_param("historyMOG2");
        subtractor.dist2ThresholdKNN = cfg_reader.read_real_param("dist2ThresholdKNN");
        subtractor.varThresholdMOG2 = cfg_reader.read_real_param("varThresholdMOG2");
    }

    void prg_config::load_cfg_source(json_t *json_upper) {
        jansson_cfg_obj_reader cfg_reader(json_upper, "source");

        source.source = cfg_reader.read_string_param("source");
        source.deviceID = cfg_reader.read_int_param("deviceID");
        source.apiID = cfg_reader.read_int_param("apiID");
        source.link = cfg_reader.read_string_param("link");
        source.frame_period = cfg_reader.read_int_param("frame_period");
    }

    void prg_config::check_cfg(prg_config &cfg) {
        // TODO
        cfg.num_frame_to_keep = cfg.median_bg.num_frame * cfg.median_bg.sampling_period + 1;
    }

	prg_config::frame_src_type prg_config::get_frame_src_type() const{
		static std::map<string, frame_src_type> known_src{
			{"campro",    fr_src_ipc},
			{"webcam",    fr_src_uri},
			{"ipcam",     fr_src_uri},
			{"videofile", fr_src_uri},
			{"imagefile", fr_src_uri},
			};

		auto it = known_src.find(source.source);
		if(it == known_src.end())
			return fr_src_invalid;
		else
			return it->second;
	}

	void prg_config::load_dp2_conn_cfg(json_t *json_upper){
		jansson_cfg_obj_reader cfg_reader(json_upper, "dp2conn");

		dp2_conn.host = cfg_reader.read_string_param("host");
		dp2_conn.port = cfg_reader.read_int_param("port");
		dp2_conn.reconn_interval_s = cfg_reader.read_int_param("reconnect_interval_s");
		if(dp2_conn.reconn_interval_s < 1)
			dp2_conn.reconn_interval_s = 3;
	}

    void prg_config::load_test_display(json_t *filters, const char *filter_name, cfg_display &res_display) {
        jansson_cfg_obj_reader cfg_reader(filters, filter_name);
        res_display.display = cfg_reader.read_bool_param("display_video");
        res_display.display_marks = cfg_reader.read_bool_param("display_marks");
        res_display.rotayte_rec = cfg_reader.read_bool_param("rotayte_rec");
        res_display.scale = cfg_reader.read_real_param("scale");
        res_display.video_save = cfg_reader.read_bool_param("video_save");
        res_display.video_name = cfg_reader.read_string_param("video_name");
        res_display.fps = cfg_reader.read_double_param("fps");
    }

    void prg_config::load_test_save_file(json_t *filters, const char *filter_name, cfg_save2file &res_file){
        jansson_cfg_obj_reader cfg_reader(filters, filter_name);

        res_file.switched = cfg_reader.read_bool_param("switched");
        res_file.time_interval = cfg_reader.read_double_param("time_interval");
        res_file.one_file = cfg_reader.read_bool_param("one_file");
        res_file.txt_file = cfg_reader.read_bool_param("txt_file");
    }

    void prg_config::load_cfg_test(json_t *json_upper){
        jansson_cfg_obj_reader cfg_reader(json_upper, "test");

        test.debug = cfg_reader.read_bool_param("test");
        test.out_folder = cfg_reader.read_string_param("out_folder");

        json_t *section_cfg = json_object_get(json_upper, "test");
        load_test_display(section_cfg, "display", test.display);
        load_test_save_file(section_cfg, "save2file", test.res_file);
    }

	void prg_config::load_cfg_calc_limits(json_t *json_upper){
		jansson_cfg_obj_reader cfg_reader(json_upper, "calc_limits");

		calc_tile_lim_cfg.enable = cfg_reader.read_bool_param("enable");
		calc_tile_lim_cfg.ref_frame_proc_time_ms = cfg_reader.read_double_param("ref_frame_proc_time_ms");

		const json_t *section_cfg = cfg_reader.read_array("levels");
		size_t index;
		json_t *value;
		json_array_foreach(section_cfg, index, value) {
			if (!json_is_array(value))
				throw std::logic_error(std::string("some of levels is not an array for calculate limits"));
			json_t *lev_arg = json_array_get(value, 0);
			if(!json_is_number(lev_arg))
				throw std::logic_error(std::string("some of arg for levels is not an number for calculate limits"));
			json_t *lev_vol = json_array_get(value, 1);
			if(!json_is_number(lev_vol))
				throw std::logic_error(std::string("some of value for levels is not an number for calculate limits"));
			calc_tile_lim_cfg.rates[json_real_value(lev_arg)] = json_real_value(lev_vol);
		}
	}
}
