#ifndef CLT_OPTIC_DP1_CONFIG_H
#define CLT_OPTIC_DP1_CONFIG_H

#include <cstdint>
#include "m_cfg_if.h"
#include <cstdint>
#include <map>
#include <vector>

struct json_t;
namespace ns_datapro1 {
	struct calc_tile_lim_cfg_t{
		bool enable;
		double ref_frame_proc_time_ms;
		std::map<double, double> rates;
	};
	class prg_config {
	public:
		struct cfg_one_filter {
			int width;
			int height;
			bool switched;
		};
        struct cfg_median_filter {
            int window_1_size;
            int window_2_size;
            bool rejection;
            bool switched;
        };
        struct cfg_display {
            bool display;
            bool display_marks;
            bool rotayte_rec;
            float scale;
            bool video_save;
            std::string video_name;
            double fps;
        };
        struct cfg_save2file {
            bool switched, one_file, txt_file;
            double time_interval;
        };
		struct cfg_multiproc{
			unsigned int tiles_factor;
			int ocv_num_thread;
			unsigned int dp1_num_thread;
		};
	private:

		void load_cfg_segment(json_t *);

		void load_cfg_filters(json_t *);

        static void load_median_filter(json_t *filters, const char *filter_name, cfg_median_filter &res_filter);

		static void load_single_filter(json_t *filters, const char *filter_name, cfg_one_filter &res_filter);

        static void load_test_display(json_t *filters, const char *filter_name, cfg_display &res_filter);

        static void load_test_save_file(json_t *filters, const char *filter_name, cfg_save2file &res_filter);

        void load_cfg_median_bg(json_t *);

        void load_cfg_subtractor(json_t *);

        void load_cfg_source(json_t *);

		void load_dp2_conn_cfg(json_t *);

        void load_cfg_test(json_t *);

		void load_cfg_calc_limits(json_t *);

		void read_cfg_multiproc(json_t *);

        void load_cfg_binocular(json_t *);

	public:
		prg_config(const char *cfg_fname, int camera_index);
        static void check_cfg(prg_config &cfg);

		enum frame_src_type
		{
			fr_src_invalid,
			fr_src_ipc,
			fr_src_uri,
		};
		[[nodiscard]] frame_src_type get_frame_src_type() const;

        struct cfg_binocular {
            std::string file_camera_settings;
            bool switched;
            int minHessian;
            bool using_template;
            std::vector<double> R_matrix;   // 9 елементів
            std::vector<double> t_vector;   // 3 елементи
        };

		struct cfg_segment {
			int element_str_w;
			int element_str_h;
			int number_iter;
			int element_str_type;
			int level;
			int level_max;
			int threshold_type;
			int contour_retrieval;
			int contour_approximation;
            int min_segment_size;
            int max_segment_size;
            double max_height_to_width_ratio;
		};
		struct cfg_filters {
			cfg_one_filter corr;
			cfg_one_filter matched;
			cfg_one_filter blur;
            cfg_median_filter median;
		};

        struct cfg_median {
            bool switched;
            int num_frame;
            int sampling_period;
            bool affine_transfor;
        };

        struct cfg_subtractor{
            bool subtractorKNN;
            bool detectShadows;
            int historyKNN;
            int historyMOG2;
            double dist2ThresholdKNN;
            double varThresholdMOG2;
        };

        struct cfg_source_frame{
            std::string source;
            int deviceID;
            int apiID;
            std::string link;
            int frame_period;
        };

		struct dp2_conn_cfg{
			std::string host;
			uint16_t port;
			int reconn_interval_s;
		};

        struct cfg_test{
            bool debug;
            std::string out_folder;
            cfg_display display;
            cfg_save2file res_file;
        };

		ipc_name_cfg ipc_cfg;
		cfg_segment segment;
		cfg_filters filters;
        cfg_median median_bg;
        cfg_subtractor subtractor;
        cfg_source_frame source;
		dp2_conn_cfg dp2_conn;
        cfg_test test;
		cfg_multiproc multiproc;
		int def_border;
		size_t num_frame_to_keep{16};
        cfg_binocular binocular;
		calc_tile_lim_cfg_t calc_tile_lim_cfg;
	};
}

#endif //CLT_OPTIC_DP1_CONFIG_H
