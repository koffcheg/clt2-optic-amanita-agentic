#include <iostream>
#include "dp1v2/startup.hpp"

int main(int argc, char *argv[]) {
    try {
        return dp1v2::run_startup(argc, argv);
    } catch (const std::exception &ex) {
        std::cerr << "error: " << ex.what() << std::endl;
        return 1;
    }
}
