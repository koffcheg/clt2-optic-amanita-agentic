

#include "args.hpp"
#include <iostream>
#include <cstring>

namespace turret {

void Args::printUsage() {
  //todo: add options:
  //  -v - verbose
  std::cout << "usage: turretstate [localhost] [9552] [turretpro-log.xml]" << std::endl;
}

bool Args::parse(int argc, const char *argv[], int &errcode) {
  if (argc == 2 && std::strcmp(argv[1], "--help") == 0) {
    printUsage();
    errcode = 0;
    return false;
  }

  if (argc >= 2) {
    infoHost = argv[1];
  }

  if (argc >= 3) {
    infoPort = argv[2];
  }
  if (argc == 4) {
    loggerConfig = argv[3];
}

  return true;
}

} // namespace turret