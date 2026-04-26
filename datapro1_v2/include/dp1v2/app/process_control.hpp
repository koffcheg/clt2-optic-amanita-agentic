#pragma once

namespace dp1v2 {

void install_process_signal_handlers();
void request_process_stop();
bool is_process_stop_requested();

} // namespace dp1v2
