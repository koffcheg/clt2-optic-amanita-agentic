#pragma once

#include <string>

namespace turret {

class Args {
public:
  // Args;
  bool parse(int argc, const char *argv[], int &errcode);
  std::string fileToOpen = "";
  std::string mainConfig = "plotter.json";
  std::string loggerConfig = "plotter-log.xml";


private:
  void printUsage();
};

} // namespace turret