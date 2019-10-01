#include "test_util_role.h"

namespace testutil {

std::ostream &operator<<(std::ostream &s, const Role &role)
{
    if (role == Role::Central)
    {
        s << "[central   ]";
    }
    else if (role == Role::Peripheral)
    {
        s << "[peripheral]";
    }
    else
    {
        s << "[unknown role]";
    }

    return s;
}
} //  namespace testutil
