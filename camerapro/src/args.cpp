

#include "args.hpp"
#include <iostream>
#include <cstring>

namespace cam_pro {

void Args::printUsage() {
  //todo: add options:
  //  -v - verbose
  std::cout << "usage: camerapro <camera_id> [camerapro.json] [camerapro-log.xml]" << std::endl;
}

bool Args::parse(int argc, const char *argv[], int &errcode) {
  if (argc == 2 && std::strcmp(argv[1], "--help") == 0) {
    printUsage();
    errcode = 0;
    return false;
  }

  if (argc <= 1) {
    printUsage();
    errcode = -3;
    return false;
  }

  if (argc >= 2) {
    // todo: check argument type???
    cameraID = std::stoi(argv[1]);
  }

  if (argc >= 3) {
    mainConfig = argv[2];
  }

  if (argc >= 4) {
    loggerConfig = argv[3];
  }

  return true;
}

} // namespace cam_pro