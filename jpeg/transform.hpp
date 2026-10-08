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
    float y = float(from[0]), cb = float(from[1]), cr = float(from[2]);
    to[0] = static_cast<uint8_t>(clamp(y + 1.402f * (cr - 128.f)));
    to[1] = static_cast<uint8_t>(clamp(y - 0.34414f * (cb - 128.f) - 0.71414f * (cr - 128.f)));
    to[2] = static_cast<uint8_t>(clamp(y + 1.772f * (cb - 128.f)));
}



// linear interpolation between values z1 and z2 with according line-coordinates x1 and x2, function finds value at line-coordinate w
// w in [x1; x2]
inline float interpolate(float x1, float x2, float z1, float z2, float w)
{
    return x1 == x2 ? z1 : (z2 - z1) * (w - x1) / (x2 - x1) + z1;
}


// linear spline between values z with according line-coordinates x, function finds value at line-coordinate w
// w in [x[0]; x[n]], x[i] <= x[i+1]
inline float spline(float* x, float* z, uint32_t n, float w)
{
    for (uint32_t i = 1; i < n; ++i)
        if (w < x[i])
            return interpolate(x[i - 1], x[i], z[i - 1], z[i], w);
    return 0.f;
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


inline void upsampling(uint8_t* orig, uint8_t* result, uint32_t width, uint32_t height, uint32_t k = 2)
{
    for (uint32_t x, y = height * k, res_width = width * k; y--;)
        for (x = width * k; x--;)
            result[x + y * res_width] = orig[(x / k) + (y / k) * width];
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
            r = min(l + 1, width - 1);//l + (l < result_width - 1);
            b = min(t + 1, height);//t + (t < result_height);

        #if USE_DEBUG_OUTPUT
            printf("[interpolation: rect %u-%u-%u-%u colors %u-%u-%u-%u -> %u-%u (inrect %f-%f) color %u]\n", l,t,r,b, orig[l + t * width], orig[r + t * width], orig[r + b * width], orig[l + b * width], u,v, x,y, uint32_t(interpolate_bilinear(l, t, r, b,
                orig[l + t * width], orig[r + t * width], orig[r + b * width], orig[l + b * width], x, y)));
        #endif

            result[(u + v * result_width) * result_struct_size + result_struct_offset] = uint8_t(interpolate_bilinear(l, t, r, b,
                orig[l + t * width], orig[r + t * width], orig[r + b * width], orig[l + b * width], x, y));
        }
}



inline void FDCT(int8_t* block, float* result, uint32_t width = 8, uint32_t height = 8)
{
#define C(c) (c ? 1. : M_SQRT1_2)
    float sum;
    for (uint32_t u = 0, v, x, y; u < width; ++u)
        for (v = 0; v < height; ++v)
        {
            for (sum = x = 0; x < width; ++x)
                for (y = 0; y < height; ++y)
                    sum += float(block[x + y * width]) * cosf((2.f * x + 1.f) * u * M_PI / (2.f * width)) * cosf((2.f * y + 1.f) * v * M_PI / (2.f * height));
            result[u + v * width] = (2.f / sqrtf(width * height)) * C(u) * C(v) * sum;
        }
}


inline void IDCT(float* block, int8_t* result, uint32_t width = 8, uint32_t height = 8)
{
    float sum;
    for (uint32_t x = 0, y, u, v; x < width; ++x)
        for (y = 0; y < height; ++y)
        {
            for (sum = u = 0; u < width; ++u)
                for (v = 0; v < height; ++v)
                    sum += C(u) * C(v) * float(block[u + v * width]) * cosf(((2.f * x + 1.f) * u * M_PI) / (2.f * width)) * cosf(((2.f * y + 1.f) * v * M_PI) / (2.f * height));
            result[x + y * width] = clamp((2.f / sqrtf(width * height)) * sum, -128, 127);
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


// block -> zigzag-ordered line of elements of that block
inline void zigzag_order(uint8_t* block, uint8_t* result, uint32_t width = 8, uint32_t height = 8)
{
    uint32_t x {}, y {}, dir { 1 };
    do
        *(result++) = block[x + y * width];
    while (zigzag(x, y, reinterpret_cast<uint8_t&>(dir), width, height));
}


// zigzag-ordered line -> block from which it was created
inline void zigzag_order_inverse(uint8_t* zigzag_order, uint8_t* block, uint32_t width = 8, uint32_t height = 8)
{
    uint32_t x {}, y {}, dir { 1 };
    do
        block[x + y * width] = *(zigzag_order++);
    while (zigzag(x, y, reinterpret_cast<uint8_t&>(dir), width, height));
}
