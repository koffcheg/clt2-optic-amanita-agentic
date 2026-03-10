
#pragma once

#include "config.hpp"
#include <memory>
#include <mutex>
#include <string>

namespace turret {

class Application {
public:
  explicit Application(const Config &config);
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;
  ~Application();
  void run();
  void terminate();

  void setFilename(const std::string &filename) { m_filename = filename; }
  const std::string &filename() { return m_filename; }

private:
  void processConnect();
  void processConnected();

  std::recursive_mutex m_mutex;

  const Config &m_config;

  std::string m_filename;
};

} // namespace turret