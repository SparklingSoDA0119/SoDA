#ifndef _SODA_TYPE_H
#define _SODA_TYPE_H

#if defined(_WIN32)
#include <ctime>
#endif

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
}       // namespace SoDA

#endif // _SODA_TYPE_H
