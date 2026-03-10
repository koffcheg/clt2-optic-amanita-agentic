

#include "infoServer.hpp"
#include "sockUtils.hpp"
#include <arpa/inet.h>
#include <fmt/format.h>
#include <linux/tcp.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <logHelper.hpp>
#include <unistd.h>

static auto logger = log4cxx::Logger::getLogger("config");

namespace turret {

InfoServer::InfoServer(const std::string &host, const std::string &port)
    : m_host(host), m_port(port), m_serverSocketfd(-1) {}

void InfoServer::run() {
  if (m_thread.joinable()) {
    // LOG4CXX_FATAL(logger, "InfoServer is already started");
    throw std::logic_error("InfoServer is already started");
  }

  m_run = true;
  m_thread = std::thread([=, this]() { threadProc(); });
}

void InfoServer::terminate() {
  m_run = false;

  if (m_thread.joinable()) {
    // LOG4CXX_INFO(logger, "wait for m_thread stop");
    m_thread.join();
    // LOG4CXX_INFO(logger, "m_thread joined");
  }
}

void InfoServer::sendData(const void *data, size_t size) {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);

  for (auto it = m_clients.begin(); it != m_clients.end();) {
    int e = checkEvents(*it, POLLOUT, 0);
    if (e > 0) {
      if (e & (POLLHUP | POLLERR)) {
        close(*it);
      }
      if (e & (POLLHUP | POLLERR | POLLNVAL)) {
        it = m_clients.erase(it);
        // todo: log InfoServer client disconnected
        continue;
      } else if (e & POLLOUT) { // verify client is ready for more data
        size_t sent = send(*it, data, size, 0);
        if (sent != size) {
          // todo: log, exception?
        }
      }
    }
    ++it;
  }
}

void InfoServer::startServer() {
  if (m_serverSocketfd != -1) {
    return; // already started
  }

  m_serverSocketfd = socket(AF_INET, SOCK_STREAM, 0);

  if (m_serverSocketfd < 0) {
    throw std::runtime_error(
        fmt::format("Can not create a socket: {}",
                    strerror(errno))); // todo: thread unsafe!
  }

  setSocketOption(m_serverSocketfd, SOL_SOCKET, SO_REUSEADDR, 1);

  struct sockaddr_in address;

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY; // todo: bind to specific address
  address.sin_port = htons(std::stoi(m_port));

  if (bind(m_serverSocketfd, reinterpret_cast<struct sockaddr *>(&address),
           sizeof address) < 0) {
    int e = errno;
    close(m_serverSocketfd);
    throw std::runtime_error(fmt::format("Can not bind the socket: {}",
                                         strerror(e))); // todo: thread unsafe!
  }

  if (listen(m_serverSocketfd, 5) < 0) {
    int e = errno;
    close(m_serverSocketfd);
    throw std::runtime_error(fmt::format("Can not listen to the socket: {}",
                                         strerror(e))); // todo: thread unsafe!
  }
}

void InfoServer::threadProc() {
  startServer();

  // todo: instant thread stop (use aux socket)
  while (m_run) {
    if (checkEvents(m_serverSocketfd, POLLIN)) {
      int clientSocket = accept(m_serverSocketfd, nullptr, nullptr);

      // todo: log new client
      disableNagle(clientSocket);

      std::lock_guard<std::recursive_mutex> guard(m_mutex);
      m_clients.push_back(clientSocket);
    }
  }
  if (close(m_serverSocketfd) < 0) {
    // todo: log
  }
}

} // namespace turret
