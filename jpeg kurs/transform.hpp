#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include "input.h"




inline void RGB_to_YCbCr(uint8_t from[3], uint8_t to[3])
{
    float r = float(from[0]), g = float(from[1]), b = float(from[2]);
    to[0] = static_cast<uint8_t>(clamp(0.299f * r + 0.587f * g + 0.114f * b));
    to[1] = static_cast<uint8_t>(clamp(-0.1687f * r - 0.3313f * g + 0.5f * b + 128.f));
    to[2] = static_cast<uint8_t>(clamp(0.5f * r - 0.4187f * g - 0.0813f * b + 128.f));
}


inline void YCbCr_to_RGB(uint8_t from[3], uint8_t to[3])
{
    float y = float(from[0]), cb = float(from[1]) - 128.f, cr = float(from[2]) - 128.f;
    to[0] = static_cast<uint8_t>(clamp(y + 1.402f * cr));
    to[1] = static_cast<uint8_t>(clamp(y - 0.34414f * cb - 0.71414f * cr));
    to[2] = static_cast<uint8_t>(clamp(y + 1.772f * cb));
}



// linear interpolation between values z1 and z2 with according line-coordinates x1 and x2, function finds value at line-coordinate w
// w in [x1; x2]
inline float interpolate(float x1, float x2, float z1, float z2, float w)
{
    return x1 == x2 ? z1 : (z2 - z1) * (w - x1) / (x2 - x1) + z1;
}


// linear interpolation on plane between values z_lt, z_rt, z_rb, z_lb with according coordinates (l; t), (r; t), (r; b), (l; b) (forming rectangular aligned with axes, "l,t,r,b" are shorts for "left, top, right, bottom", orientation from left to right and top to bottom), function finds value at coordinate (x; y)
// x in [l; r], y in [t; b]
inline float interpolate_bilinear(float l, float t, float r, float b, float z_lt, float z_rt, float z_rb, float z_lb, float x, float y)
{
    return interpolate(t, b, interpolate(l, r, z_lt, z_rt, x), interpolate(l, r, z_lb, z_rb, x), y);
}



inline void downsampling(uint8_t* orig, uint8_t* result, uint32_t width, uint32_t height, uint32_t k = 2)
{
    height /= k;
    for (uint32_t res_x, res_y = 0, orig_x, orig_y, orig_xm, orig_ym, sum, res_width = width / k; res_y < height; ++res_y)
        for (res_x = 0; res_x < res_width; ++res_x)
        {
            for ((sum = 0), (orig_ym = (orig_y = res_y * k) + k); orig_y < orig_ym; ++orig_y)
                for (orig_xm = (orig_x = res_x * k) + k; orig_x < orig_xm; ++orig_x)
                    sum += orig[orig_x + orig_y * width];
            result[res_x + res_y * res_width] = sum / k / k;
        }
}



// result consists of structs of size result_struct_size and resized image will be written into these structs on result_struct_offset byte in each
inline void resize(uint8_t* orig, uint32_t width, uint32_t height, uint8_t* result, uint32_t result_width, uint32_t result_height, uint8_t result_struct_offset = 0, uint8_t result_struct_size = 1)
{
    //float sx = (width - 1) / (result_width - 1), sy = (--height) / (result_height - 1), x, y;
    float sx = float(width) / result_width, sy = float(height--) / result_height, x, y;
    for (uint32_t u = 0, v, l, t, r, b; u < result_width; ++u)
        for (v = 0; v < result_height; ++v)
        {
            l = uint32_t(x = float(u) * sx);
            t = uint32_t(y = float(v) * sy);
            r = l + (l < result_width - 1);
            b = t + (t < result_height);
            result[(u + v * result_width) * result_struct_size + result_struct_offset] = uint8_t(interpolate_bilinear(l, t, r, b,
                orig[l + t * width], orig[r + t * width], orig[r + b * width], orig[l + b * width], x, y));
        }
}



inline void FDCT_Q(int8_t* block, int16_t* result, uint8_t* QTable)
{
#define C(c) (c ? 1. : M_SQRT1_2)
    float sum;
    for (uint32_t u = 0, v, x, y; u < 8; ++u)
        for (v = 0; v < 8; ++v)
        {
            for (sum = x = 0; x < 8; ++x)
                for (y = 0; y < 8; ++y)
                    sum += float(block[x + y * 8]) * cosf((2.f * x + 1.f) * u * (M_PI / 16.f)) * cosf((2.f * y + 1.f) * v * (M_PI / 16.f));
            result[u + v * 8] = roundf(C(u) * C(v) * sum / 4.f / float(QTable[u + v * 8]));
        }
}


inline void IDCT(int16_t* block, int8_t* result)
{
    float sum;
    for (uint32_t x = 0, y, u, v; x < 8; ++x)
        for (y = 0; y < 8; ++y)
        {
            for (sum = u = 0; u < 8; ++u)
                for (v = 0; v < 8; ++v)
                    sum += C(u) * C(v) * float(block[u + v * 8]) * cosf((2.f * x + 1.f) * u * (M_PI / 16.f)) * cosf((2.f * y + 1.f) * v * (M_PI / 16.f));
            result[x + y * 8] = clamp(sum / 4.f, -128, 127);
        }
#undef C
}



// initialize x=y=0, dir=1 (up-right) or dir=0 (down-left) and perform actions on (x;y) before calling this function
// returns false if zigzag was completed (no more elements to process in zigzag-order), true if (x;y) was moved to the next zigzag order element
// x in [0; width), y in [0; height), dir is boolean
inline bool zigzag(uint32_t& x, uint32_t& y, uint8_t& dir, uint32_t width = 8, uint32_t height = 8)
{
    if (dir)
    {
        if (++x < width)
        {
            if (y)
                --y;
            else
                dir = 0;
        }
        else if (++y < height)
        {
            --x;
            dir = 0;
        }
        else
            return false;
    }
    else
    {
        if (++y < height)
        {
            if (x)
                --x;
            else
                dir = 1;
        }
        else if (++x < width)
        {
            --y;
            dir = 1;
        }
        else
            return false;
    }
    return true;
}
