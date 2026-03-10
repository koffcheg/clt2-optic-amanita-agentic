//
// Created by u on 24.07.24.
//

#ifndef CLT_OPTIC_DP1_RPC_DEFINES_H
#define CLT_OPTIC_DP1_RPC_DEFINES_H
#include <cstdint>

const uint16_t dp1_to_dp2_rpc_msg_new_measure = 1;
const uint16_t dp1_to_dp2_camera_calibration_data = 2;	//ДП1 передає калібровочні налаштування камери на ДП2
const uint16_t dp2_to_turret_rpc_msg_tracks = 10;		//ДП2 передає поточні траєкторіх на турель
const uint16_t dp2_to_turret_rpc_msg_new_tracks = 11;		//ДП2 передає на турель нові траекторії
const uint16_t dp2_to_turret_rpc_msg_update_tracks = 12;		//ДП2 передає на турель оновлення по траекторіям
const uint16_t dp2_to_turret_rpc_msg_dropped_tracks = 13;		//ДП2 передає на турель траекторії, що перестали існувати
#endif //CLT_OPTIC_DP1_RPC_DEFINES_H
