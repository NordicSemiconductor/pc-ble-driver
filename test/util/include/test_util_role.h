#pragma once

#include "ble.h"
#include <iostream>

namespace testutil {
enum Role { Central = BLE_GAP_ROLE_CENTRAL, Peripheral = BLE_GAP_ROLE_PERIPH };

std::ostream &operator<<(std::ostream &s, const Role &role);
} //  namespace testutil
