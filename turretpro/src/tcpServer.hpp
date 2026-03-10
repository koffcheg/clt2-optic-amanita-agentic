

#pragma once

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

namespace turret {

class TcpServerClient {
public:
  TcpServerClient(int socketfd);
  virtual ~TcpServerClient();
  virtual void disconnect();
  // todo: doesn't it worth to do the next function protected????
  void sendData(const void *data, size_t size);
  bool running() { return m_run; }
  void stop();

protected:
  virtual void processData(const void *data, size_t size);

private:
  void threadProc();

  std::atomic_bool m_run;
  std::thread m_thread;
  std::recursive_mutex m_mutex;
  int m_socketfd;
};

// template <class T>
class TcpServer {
public:
  TcpServer(const std::string &host, const std::string &port);
  void run();
  void terminate();

protected:
  virtual std::unique_ptr<TcpServerClient> newClient(int socket) const;

private:
  void startServer();
  void threadProc();
  void removeDisconnectedClients();

  std::string m_host;
  std::string m_port;
  int m_serverSocketfd;

  std::list<std::unique_ptr<TcpServerClient>> m_clients;

  std::atomic_bool m_run;
  std::thread m_thread;
  std::recursive_mutex m_mutex;
};

} // namespace turret