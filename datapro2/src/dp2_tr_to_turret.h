//
// Created by u on 28.07.24.
//

#ifndef CLT_OPTIC_DP2_TR_TO_TURRET_H
#define CLT_OPTIC_DP2_TR_TO_TURRET_H

namespace boost::asio{class io_context;}
namespace dp2{struct dp2_cfg;}
class StrobeMethod;

void init_tr_to_turret(boost::asio::io_context &, const dp2::dp2_cfg &);

void tr_updates_to_turret(const StrobeMethod *, unsigned int spent_proc_time);

#endif //CLT_OPTIC_DP2_TR_TO_TURRET_H
