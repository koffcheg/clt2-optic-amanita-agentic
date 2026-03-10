//
// Created by u on 27.07.24.
//

#ifndef CLT_OPTIC_DP2_RPC_DATA_MRSH_H
#define CLT_OPTIC_DP2_RPC_DATA_MRSH_H
class CMemStore;
struct Trajectory;

void serialize_dp2_res(CMemStore &, const Trajectory&);
bool deserialize_dp2_res(CMemStore &, Trajectory&);

#endif //CLT_OPTIC_DP2_RPC_DATA_MRSH_H
