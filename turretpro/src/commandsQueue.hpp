
#pragma once

#include "turretCommands.hpp"
#include "sharedQueue.hpp"

namespace turret{

using CommandsQueue = SharedQueue<CommandPtr>;

}



