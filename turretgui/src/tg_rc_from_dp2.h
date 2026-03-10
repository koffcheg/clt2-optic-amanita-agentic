//
// Created by merich on 28.07.24.
//

#ifndef CLT_OPTIC_TG_RC_FROM_DP2_H
#define CLT_OPTIC_TG_RC_FROM_DP2_H

#include <string>
#include <map>
#include <vector>
#include "datapro2/src/datapro2Types.h"

//тип для callback-функції для сповіщення про нові траекторії або про оновлення існуючих траекторій
using dp2_track_upd_func_notify_t = void (*)(int dp2_id, unsigned int dp12_proc_delay_ms, const std::vector<Trajectory> &);

//тип для callback-функції для сповіщення про скидання траекторій
using dp2_track_drop_func_notify_t = void (*)(int dp2_id, const std::vector<unsigned long> &);

void init_rc_data_from_dp2(uint16_t listen_port);

void stop_rc_data_from_dp2();

std::map<int, std::vector<Trajectory>> get_curr_tracks_from_dp2();	//dp2 --> dp2's tracks

//реєстрація callback функції, яка буде викликатись при оновлені траекторій в ДП2
void register_dp2_upd_tracks_callback(dp2_track_upd_func_notify_t );

//реєстрація callback функції, яка буде викликатись при "створенні" нових траекторій в ДП2
void register_dp2_new_tracks_callback(dp2_track_upd_func_notify_t );

//реєстрація callback функції, яка буде викликатись при "скиданні" траекторій в ДП2
void register_dp2_drop_tracks_callback(dp2_track_drop_func_notify_t );

#endif //CLT_OPTIC_TG_RC_FROM_DP2_H
