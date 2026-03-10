#include "fileLogger.hpp"
#include "fmt/chrono.h"
#include "fmt/format.h"
#include <sstream>

//todo: create file only if enabled!
FileLogger::FileLogger(const std::string &filename, const std::string &format,
                       bool enabled)
    : m_enabled(enabled), m_outfile(filename) {
  if (m_outfile.is_open() && m_enabled) {
    if (!format.empty()) {
      m_outfile << format << std::endl;
    }
  }
}
void FileLogger::log(std::chrono::system_clock::time_point t,
                     const std::vector<double> &v) {
  if (!m_enabled) {
    return;
  }
  std::ostringstream oss;
  oss << fmt::format("{:%Y.%m.%d %H:%M:%S} ", t);
  for (auto d : v) {
    oss << d << " ";
  }
  m_outfile << oss.str() << std::endl;
};

void FileLogger::logstr(std::chrono::system_clock::time_point t,
                        const std::string &s) {
  if (!m_enabled) {
    return;
  }
  std::ostringstream oss;
  oss << fmt::format("{:%Y.%m.%d %H:%M:%S} ", t);
  oss << s;
  m_outfile << oss.str() << std::endl;
};
