
#pragma once

#include <poll.h>

namespace turret {


/// @brief Default poll timeout.
/// @note Large values will extend thread termination time in the current implementation.
constexpr int POLL_TIMEOUT = 100;

int checkEvents(int socket, int events, int timout = POLL_TIMEOUT);

/// @brief set socket option
/// @param socket
/// @param level
/// @param option
/// @param value
/// @return -1 in a case of an error
int setSocketOption(int socket, int level, int option, int value);

/// @brief  Disables Nagle's algorithm
/// @param socket
/// @return -1 in a case of an error
int disableNagle(int socket);

} // namespace turret