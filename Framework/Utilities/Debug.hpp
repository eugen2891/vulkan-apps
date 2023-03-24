#pragma once

#include <cstdlib>

#ifdef _DEBUG

#define ReturnIfNot(expr) { \
	if (!(expr)) { __debugbreak(); return; } \
}

#define RetvalIfNot(expr, val) { \
	if (!(expr)) { __debugbreak(); return (val); }\
}

#define BreakIfNot(expr) { \
	if (!(expr)) { __debugbreak(); abort(); } \
}

#else

#define ReturnIfNot(expr) { \
	if (!(expr)) return; \
}

#define RetvalIfNot(expr, val) { \
	if (!(expr)) return (val); \
}

#define BreakIfNot(expr) (expr)

#endif

#define Min(a,b) (((a)<(b))?(a):(b))
#define Max(a,b) (((a)>(b))?(a):(b))
