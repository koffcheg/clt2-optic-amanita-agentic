

#include "tcpClient.hpp"
#include "sockUtils.hpp"
#include <arpa/inet.h>
//#include <format>
#include <linux/tcp.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace turret {

static constexpr int BUFF_SIZE = 16384;

/// @brief Time to sleep between checking reconnect timeout
/// @details Granularity of reconnect timeout. Larger value means more time to
/// wait to stop connecting thread.
static constexpr std::chrono::milliseconds RECONNECT_SLEEP(100);

// TcpClient

TcpClient::TcpClient(const std::string &address, const std::string &port,
                     int reconnectIntervalms, int dataTimeout)
    : m_address(address), m_port(port), m_clientSocketfd(-1), m_run{false},
      m_connected{false}, m_reconnectIntervalms{reconnectIntervalms},
      m_dataTimeout(dataTimeout) {}

TcpClient::~TcpClient() { terminate(); }

void TcpClient::run() {
  if (m_thread.joinable()) {
    // LOG4CXX_FATAL(logger, "TcpClient is already started");
    throw std::logic_error("TcpClient is already started");
  }

  m_run = true;
  m_thread = std::thread([=, this]() { threadProc(); });
}

void TcpClient::terminate() {
  m_run = false;

  if (m_thread.joinable()) {
    // LOG4CXX_INFO(logger, "wait for m_thread stop");
    m_thread.join();
    // LOG4CXX_INFO(logger, "m_thread joined");
  }
}

void TcpClient::sendData(const void *data, size_t size) {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);

  if (!m_connected) {
    // todo: log
    throw std::runtime_error("Client not connected");
  }

  int e = checkEvents(m_clientSocketfd, POLLOUT, 0);
  if (e > 0) {
    if (e & (POLLHUP | POLLERR | POLLNVAL)) {
      // todo: log TcpClient client disconnected
      disconnect();
    }
    if (e & POLLOUT) { // verify client is ready for more data
      size_t sent = send(m_clientSocketfd, data, size, 0);
      if (sent != size) {
        // todo: log, exception?
      }
    }
  }
  if (e < 0) {
    // todo:
  }
}

void TcpClient::disconnect() {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  // todo: log TcpClient client disconnected
  if (m_clientSocketfd != -1) {
    close(m_clientSocketfd);
    m_clientSocketfd = -1;
  }
  m_connected = false;
}

void TcpClient::tryConnect() {
  std::lock_guard<std::recursive_mutex> guard(m_mutex);
  if (m_clientSocketfd != -1) {
    return; // already connected
  }

  if (std::chrono::steady_clock::now() - m_lastConnectAttempt <
      std::chrono::milliseconds(m_reconnectIntervalms)) {
    std::this_thread::sleep_for(RECONNECT_SLEEP);
    return;
  }

  struct addrinfo hints;
  struct addrinfo *result, *rp;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM; /* Stream socket (TCP)*/
  hints.ai_flags = 0;
  hints.ai_protocol = 0; /* Any protocol */

  int s = getaddrinfo(m_address.c_str(), m_port.c_str(), &hints, &result);
  if (s != 0) {
    // todo: log fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return;
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    m_clientSocketfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

    disableNagle(m_clientSocketfd);

    if (m_clientSocketfd == -1)
      continue;

    if (connect(m_clientSocketfd, rp->ai_addr, rp->ai_addrlen) != -1) {
      // Success
      m_connected = true;
      break;
    }

    close(m_clientSocketfd);
    m_clientSocketfd = -1;
  }

  if (m_clientSocketfd == -1) {
    // todo log: std::format("Can not connect to {}:{}", ));
    m_lastConnectAttempt = std::chrono::steady_clock::now();
  }

  freeaddrinfo(result);
}

void TcpClient::processData(const void *, size_t) {}

void TcpClient::processConnected() {
  using namespace std::chrono;
  char buff[BUFF_SIZE];
  int events = checkEvents(m_clientSocketfd, POLLIN);
  if (events & (POLLERR | POLLHUP | POLLNVAL)) {
    disconnect();
  } else if (events & POLLIN) {
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    int read = recv(m_clientSocketfd, buff, sizeof(buff), MSG_DONTWAIT);
    if (read > 0) {
      m_lastDataReceived = steady_clock::now();
      processData(buff, read);
    } else if (read == 0) {
      if (m_dataTimeout && steady_clock::now() - m_lastDataReceived >
                               milliseconds(m_dataTimeout)) {
        disconnect();
      }
    } else if (read < 0) {
      disconnect();
    }
  }
}

void TcpClient::threadProc() {
  // todo: instant thread stop (use aux socket)
  while (m_run) {
    if (!m_connected) {
      tryConnect();
    } else {
      processConnected();
    }
  }
  if (close(m_clientSocketfd) < 0) {
    // todo: log
  }
}

} // namespace turret
