#include <log4cxx/logger.h>

#include "dp1v2/app/process_control.hpp"
#include "dp1v2/app/startup.hpp"

int main(int argc, char *argv[]) {
    try {
        dp1v2::install_process_signal_handlers();
        return dp1v2::run_startup(argc, argv);
    } catch (const std::exception &ex) {
        LOG4CXX_ERROR(
            log4cxx::Logger::getLogger("amanita.dp1.runtime"),
            "event=bootstrap_failed status=failed reason=" << ex.what() << " exit_code=1");
        return 1;
    }
}
