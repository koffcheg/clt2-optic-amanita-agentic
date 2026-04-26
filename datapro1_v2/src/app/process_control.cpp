#include "dp1v2/app/process_control.hpp"

#include <csignal>

namespace {

volatile std::sig_atomic_t g_process_stop_requested = 0;

void handle_stop_signal(int) {
    g_process_stop_requested = 1;
}

} // namespace

namespace dp1v2 {

void install_process_signal_handlers() {
    std::signal(SIGINT, handle_stop_signal);
    std::signal(SIGTERM, handle_stop_signal);
}

void request_process_stop() {
    g_process_stop_requested = 1;
}

bool is_process_stop_requested() {
    return g_process_stop_requested != 0;
}

} // namespace dp1v2

bool is_program_stop() {
    return dp1v2::is_process_stop_requested();
}
