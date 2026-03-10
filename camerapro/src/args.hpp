#pragma once

#include <string>

namespace cam_pro {

class Args {
public:
  // Args;
  bool parse(int argc, const char *argv[], int &errcode);
  int cameraID = -1;
  std::string mainConfig = "camerapro.json";
  std::string loggerConfig = "camerapro-log.xml";

private:
  void printUsage();
};

} // namespace cam_pro