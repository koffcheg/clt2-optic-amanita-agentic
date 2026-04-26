#include <iostream>

#include "dp1v2/app/process_control.hpp"
#include "dp1v2/app/startup.hpp"

int main(int argc, char *argv[]) {
    try {
        dp1v2::install_process_signal_handlers();
        return dp1v2::run_startup(argc, argv);
    } catch (const std::exception &ex) {
        std::cerr << "error: " << ex.what() << std::endl;
        return 1;
    }
}
