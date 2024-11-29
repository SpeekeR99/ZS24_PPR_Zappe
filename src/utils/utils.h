#pragma once

/* Decimal type, can be changed by user to float, by default it is double */
#ifdef _USE_FLOAT
using decimal = float;
#else
using decimal = double;
#endif

/* Un-define max and min macros, so that std::max, std::min and limits can be used */
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

/* OpenCL library */
#ifdef _MSC_VER
#pragma comment(lib, "opencl.lib")
#endif
