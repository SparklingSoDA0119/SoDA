#ifndef _SODA_TYPE_H_
#define _SODA_TYPE_H_

#if defined(_WIN32)
	#include <ctime>
#endif

# pragma once

namespace SoDA {
	typedef   signed char  int8;
    typedef unsigned char  uint8;
    typedef   signed short int16;
    typedef unsigned short uint16;
    typedef   signed int   int32;
    typedef unsigned int   uint32;

#if defined(_WIN32)
    typedef signed   __int64  int64;
    typedef unsigned __int64  uint64;
#else
    typedef   signed long long int64;
    typedef unsigned long long int64;
#endif

    typedef float  float32;
    typedef double float64;

#if defined(_WIN32)
    typedef     __time32_t  time_t32;
    typedef     __time64_t  time_t64;
#else
    typedef          int32  time_t32;
    typedef          int64  time_t64;
#endif

    template<class T>
    inline T absDiff(const T& a, const T& b) { return  (a < b) ? (b - a) : (a - b); }

} // namespace SoDA
#endif // _SODA_TYPE_H_
