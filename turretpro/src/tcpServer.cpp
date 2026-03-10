

#include "tcpServer.hpp"
#include "sockUtils.hpp"
#include <arpa/inet.h>
#include <fmt/format.h>
#include <linux/tcp.h>
#include <logHelper.hpp>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace turret {

static auto logger = log4cxx::Logger::getLogger("tcpServer");

static constexpr size_t RX_BUFF_SIZE = 16384;

// TCP server client

TcpServerClient::TcpServerClient(int socketfd)
    : m_run(true), m_thread{std::thread([=, this]() { threadProc(); })},
      m_socketfd(socketfd) {
  disableNagle(socketfd);
}

TcpServerClient::~TcpServerClient() {
  m_run = false;
  if (m_thread.joinable()) {
    LOG_DEBUG("Waiting for TcpServerClient thread to stop");
    m_thread.join();
    LOG_INFO("TcpServerClient thread joined");
  }
}

void TcpServerClient::sendData(const void *data, size_t size) {
  if (!m_run) {
    LOG_ERROR("Can not send data. Client not connected");
    throw std::runtime_error("Client not connected");
  }

  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  int e = checkEvents(m_socketfd, POLLOUT, 0);
  if (e > 0) {
    if (e & (POLLHUP | POLLERR | POLLNVAL)) {
      // todo: log TcpClient client disconnected reason
      // LOG_INFO("Client disconnected: " << (e&POLLHUP?"POLLHUP ":" ") <<
      // e&POLLERR?"POLLERR ":" " << e&POLLNVAL?"POLLNVAL":"");
      disconnect();
    }
    if (e & POLLOUT) { // verify client is ready for more data
      size_t sent = send(m_socketfd, data, size, 0);
      if (sent != size) {
        // todo: log, exception?
      }
    }
  }
  if (e < 0) {
    // todo:
  }
}

void TcpServerClient::processData(const void * /*data*/, size_t /*size*/) {}

void TcpServerClient::disconnect() {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  // todo: log TcpClient client disconnected
  if (m_socketfd != -1) {
    close(m_socketfd);
    m_socketfd = -1;
  }
  m_run = false;
}

void TcpServerClient::threadProc() {

  while (m_run) {
    char buff[RX_BUFF_SIZE];
    int events = checkEvents(m_socketfd, POLLIN);
    if (events & (POLLERR | POLLHUP | POLLNVAL)) {
      disconnect();
    } else if (events & POLLIN) {
      std::lock_guard<std::recursive_mutex> guard(m_mutex);
      int read = recv(m_socketfd, buff, sizeof(buff), MSG_DONTWAIT);
      if (read > 0) {
        processData(buff, read);
      } else if (read == 0) {
        // todo: log graceful disconnect
        disconnect();
      } else if (read < 0) {
        // todo: log error
        disconnect();
      }
    }
  }
  disconnect();
}

void TcpServerClient::stop() {
  m_run = false;
  if (m_thread.joinable()) {
    m_thread.join();
  }
}

// TCP server

TcpServer::TcpServer(const std::string &host, const std::string &port)
    : m_host(host), m_port(port), m_serverSocketfd(-1) {}

static constexpr int CLIENT_QUEUE_LENGTH = 5;

void TcpServer::run() {
  if (m_thread.joinable()) {
    // LOG4CXX_FATAL(logger, "TcpServer is already started");
    throw std::logic_error("TcpServer is already started");
  }

  m_run = true;
  m_thread = std::thread([=, this]() { threadProc(); });
}

void TcpServer::removeDisconnectedClients() {
  for (auto it = m_clients.begin(); it != m_clients.end();) {
    if (!(*it)->running()) {
      (*it)->stop(); // waits for thread ends
      it = m_clients.erase(it);
      // todo: log InfoServer client disconnected
    }
    ++it;
  }
}

void TcpServer::terminate() {
  m_run = false;
  if (m_thread.joinable()) {
    // LOG4CXX_INFO(logger, "wait for m_thread stop");
    m_thread.join();
    // LOG4CXX_INFO(logger, "m_thread joined");
  }
}

void TcpServer::startServer() {
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

  if (listen(m_serverSocketfd, CLIENT_QUEUE_LENGTH) < 0) {
    int e = errno;
    close(m_serverSocketfd);
    throw std::runtime_error(fmt::format("Can not listen to the socket: {}",
                                         strerror(e))); // todo: thread unsafe!
  }
}

std::unique_ptr<TcpServerClient> TcpServer::newClient(int socket) const {
  return std::make_unique<TcpServerClient>(socket);
}

void TcpServer::threadProc() {
  // Listening socket thread
  startServer();

  // todo: instant thread stop (use aux socket)
  while (m_run) {
    if (checkEvents(m_serverSocketfd, POLLIN)) {
      int clientSocket = accept(m_serverSocketfd, nullptr, nullptr);

      // todo: log new client

      std::lock_guard<std::recursive_mutex> guard(m_mutex);
      m_clients.push_back(newClient(clientSocket));
    }
    removeDisconnectedClients();
  }

  if (close(m_serverSocketfd) < 0) {
    // todo: log
  }

  for (auto &client : m_clients) {
    client->disconnect();
  }
  removeDisconnectedClients();
}

} // namespace turret
