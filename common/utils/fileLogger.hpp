
#pragma once

#include <chrono>
#include <fstream>
#include <string>
#include <vector>

class FileLogger {
public:
  FileLogger(const std::string &filename, const std::string &format, bool enabled);
  void log(std::chrono::system_clock::time_point t,
           const std::vector<double> &v);
  void logstr(std::chrono::system_clock::time_point t, const std::string &s);

protected:
  bool m_enabled;

private:
  std::ofstream m_outfile;
};
