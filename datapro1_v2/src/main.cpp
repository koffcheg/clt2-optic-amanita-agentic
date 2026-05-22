#include <log4cxx/logger.h>

#include <iostream>

#include "dp1v2/app/process_control.hpp"
#include "dp1v2/app/startup.hpp"
#include "dp1v2/runtime/log_field_sanitizer.hpp"

int main(int argc, char *argv[]) {
    try {
        dp1v2::install_process_signal_handlers();
        return dp1v2::run_startup(argc, argv);
    } catch (const std::exception &ex) {
        const std::string reason = dp1v2::sanitize_log_field(ex.what());
        LOG4CXX_ERROR(
            log4cxx::Logger::getLogger("amanita.dp1.runtime"),
            "event=bootstrap_failed status=failed reason=" << reason << " exit_code=1");
        std::cerr << "event=bootstrap_failed status=failed reason=" << reason
                  << " exit_code=1" << '\n';
        return 1;
    }
}
