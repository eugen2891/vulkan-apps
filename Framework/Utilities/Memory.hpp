#pragma once

#define KB <<10
#define MB <<20
#define GB <<30

template <typename T>
inline constexpr T AlignNearest(T val, T boundary)
{
	return (val) ? ((val - 1) / boundary + 1) * boundary : 0;
}

template <typename T>
inline constexpr T AlignUp(T val, T boundary)
{
	return val ? ((val+boundary-1) - ((val-1) % boundary)) : 0;
}
