#pragma once

#include "register.h"

enum class EventRegister : uint8_t {
    ENTRY = 2,
    CMD_PARAMETERS,
    REMOVE,
    REMOVE_ALL
};