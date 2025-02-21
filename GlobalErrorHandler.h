#pragma once

#include "ACSFormat/Structs.h"
#include <stacktrace>

namespace ErrorHandler {
	void CheckSDLError(bool ret, const char* str = __builtin_FUNCTION());
}