#include "sockUtils.hpp"
#include <linux/tcp.h>
#include <poll.h>
#include <sys/socket.h>
// #include <sys/types.h>
#include <arpa/inet.h>

namespace turret {

// todo:
int checkEvents(int socket, int events, int timout) {
  struct pollfd fds;
  fds.fd = socket;
  fds.events = events;
  int ready = poll(&fds, 1, timout);

  if (ready < 0) {
    return -1;
  } else if (ready == 0) {
    return 0;
  }

  return fds.revents;
}

int setSocketOption(int socket, int level, int option, int value) {
  return setsockopt(socket, level, option, &value, sizeof(value));
}

int disableNagle(int socket) {
  return setSocketOption(socket, SOL_SOCKET, TCP_NODELAY, 1);
}

} // namespace turret