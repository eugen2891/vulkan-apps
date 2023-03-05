#pragma once

#include <cstdlib>

#define ReturnIfNot(expr) { \
	if (!(expr)) { __debugbreak(); return; } \
}

#define RetvalIfNot(expr, val) { \
	if (!(expr)) { __debugbreak(); return (val); }\
}

#define BreakIfNot(expr) { \
	if (!(expr)) { __debugbreak(); abort(); } \
}

#define Min(a,b) (((a)<(b))?(a):(b))
#define Max(a,b) (((a)>(b))?(a):(b))
