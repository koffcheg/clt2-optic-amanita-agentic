#pragma once

#include <string>

namespace turret {

class Args {
public:
  // Args;
  bool parse(int argc, const char *argv[], int &errcode);
  std::string loggerConfig = "turretstate-log.xml";
  std::string infoHost = "localhost";
  std::string infoPort = "9552";


private:
  void printUsage();
};

} // namespace turret