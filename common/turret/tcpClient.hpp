

#pragma once

#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

namespace turret {

class TcpClient {
public:
  TcpClient(const std::string &address, const std::string &port,
            int reconnectIntervalms = 1000, int dataTimeout = 0);
  virtual ~TcpClient();
  void run();
  void terminate();

  void sendData(const void *data, size_t size);

  bool connected() { return m_connected; }

protected:
  virtual void processData(const void *data, size_t size);

private:
  void disconnect();
  void tryConnect();
  void processConnected();
  void threadProc();

  std::string m_address;
  std::string m_port;

  int m_clientSocketfd;

  std::atomic_bool m_run;
  std::atomic_bool m_connected;
  std::thread m_thread;
  std::recursive_mutex m_mutex;

  int m_reconnectIntervalms;
  int m_dataTimeout;
  std::chrono::time_point<std::chrono::steady_clock> m_lastConnectAttempt;
  std::chrono::time_point<std::chrono::steady_clock> m_lastDataReceived;
};

} // namespace turret