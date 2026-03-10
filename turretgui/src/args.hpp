#pragma once

#include <string>

namespace turret {

class Args {
public:
  // Args;
  bool parse(int argc, const char *argv[], int &errcode);
  std::string mainConfig = "turretgui.json";
  std::string loggerConfig = "turretgui-log.xml";


private:
  void printUsage();
};

} // namespace turret