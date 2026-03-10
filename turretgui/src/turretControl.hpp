

#pragma once

#include "application.hpp"
#include <cstdint>

void turretControlInit(uint16_t port, turret::Application &application);
void turretControlStop();

