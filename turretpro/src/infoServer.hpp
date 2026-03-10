

#pragma once

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

namespace turret {

class InfoServer {
public:
  InfoServer(const std::string &host, const std::string &port);
  void run();
  void terminate();

  void sendData(const void *data, size_t size);

private:
  void startServer();
  void threadProc();

  std::string m_host;
  std::string m_port;
  int m_serverSocketfd;

  std::list<int> m_clients;

  std::atomic_bool m_run;
  std::thread m_thread;
  std::recursive_mutex m_mutex;
};

} // namespace turret