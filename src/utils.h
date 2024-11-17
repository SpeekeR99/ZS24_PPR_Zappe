#pragma once

/** Max byte value */
constexpr size_t max_byte_value = 256;
/** 1 KB */
constexpr size_t KB = 1 << 10;
/** 1 MB */
constexpr size_t MB = KB << 10;

/* TODO: Maybe user can choose to use floats instead of double precision ? */

/** Decimal type, can be changed by user to float */
using decimal = double;
