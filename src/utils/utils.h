#pragma once

/** Decimal type, can be changed by user to float, by default it is double */
#ifdef _USE_FLOAT
using decimal = float;
#else
using decimal = double;
#endif
