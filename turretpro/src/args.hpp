#pragma once

#include <string>

namespace turret {

class Args {
public:
  // Args;
  bool parse(int argc, const char *argv[], int &errcode);
  std::string mainConfig = "turretpro.json";
  std::string loggerConfig = "turretpro-log.xml";

private:
  void printUsage();
};

} // namespace turret