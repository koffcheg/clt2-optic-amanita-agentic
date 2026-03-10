#ifndef CLT_OPTIC_DP1M_RECEIVER_H
#define CLT_OPTIC_DP1M_RECEIVER_H
namespace ns_dp1mock_rc{
	void init_logger(int cam_index);
	void start_receive(const char *cfg_fname, int camera_index, bool (*need_stop)());
}
#endif //CLT_OPTIC_DP1M_RECEIVER_H
