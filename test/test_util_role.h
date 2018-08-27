#ifndef TEST_UTIL_ROLE_H__
#define TEST_UTIL_ROLE_H__

#include "ble.h"

namespace testutil {
enum Role { Central = BLE_GAP_ROLE_CENTRAL, Peripheral = BLE_GAP_ROLE_PERIPH };

std::ostream &operator<<(std::ostream &s, const Role &role) {
    if (role == Role::Central)
    {
        s << "[central   ]";
    }
    else if (role == Role::Peripheral)
    { s << "[peripheral]"; }
    else
    { s << "[unknown role]"; }

    return s;
}
} //  namespace testutil

#endif // TEST_UTIL_ROLE_H__