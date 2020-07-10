#pragma once

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ == 201112L)
#include <stdalign.h>
#else
#define alignas(B) __declspec(align( B ))
#endif

typedef struct vec4f
{
    alignas(16) float v[4];
} vec4f;

typedef struct vec3f
{
    alignas(16) float v[3];
} vec3f;

float dot3f(const vec3f* a, const vec3f* b)
{
    return a->v[0] * b->v[0]
         + a->v[1] * b->v[1]
         + a->v[2] * b->v[2];
}

float dot4f(const vec4f* a, const vec4f* b)
{
    return a->v[0] * b->v[0]
         + a->v[1] * b->v[1]
         + a->v[2] * b->v[2]
         + a->v[3] * b->v[3];
}
