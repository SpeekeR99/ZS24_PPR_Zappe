#pragma once

/* Common utilities */

/** Max byte value */
constexpr size_t max_byte_value = 256;
/** 1 KB */
constexpr size_t KB = 1 << 10;
/** 1 MB */
constexpr size_t MB = KB << 10;

/* Drawing utilities */

/** Padding for the plot */
constexpr size_t padding = 50;
/** Canvas width */
constexpr size_t canvas_width = 800.0;
/** Canvas height */
constexpr size_t canvas_height = 600.0;
/** Plot area width */
constexpr size_t plot_width = canvas_width - 2 * padding;
/** Plot area height */
constexpr size_t plot_height = canvas_height - 2 * padding;
/** Number of ticks on each axis */
constexpr size_t num_ticks = 10;

/** White color */
constexpr size_t white = 0xffffff;
/** Gray color (grid) */
constexpr size_t gray = 0xc8c8c8;
/** Black color */
constexpr size_t black = 0x000000;
/** Red color */
constexpr size_t red = 0xff0000;
/** Green color */
constexpr size_t green = 0x00ff00;
/** Blue color */
constexpr size_t blue = 0x0000ff;
/** Magenta color */
constexpr size_t magenta = 0xff00ff;
/** Yellow color */
constexpr size_t yellow = 0xffff00;
/** Cyan color */
constexpr size_t cyan = 0x00ffff;

/* Decimal type definition */

/** Decimal type, can be changed by user to float, by default it is double */
#ifdef _USE_FLOAT
using decimal = float;
#else
using decimal = double;
#endif
