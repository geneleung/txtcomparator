#ifndef SUPPORT_DATATYPES_H
#define SUPPORT_DATATYPES_H

#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1
#define HAVE_UINT64_T 1


#ifndef _MSC_VER

/* Note that <inttypes.h> includes <stdint.h>, if this is a C99 system. */
#include <sys/types.h>

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#else /* _MSC_VER */
/* Visual C++ doesn't provide standard integer headers, but it does provide
    built-in data types. */
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
#if defined(_WIN64)
typedef signed __int64 ssize_t;
#else
typedef signed int ssize_t;
#endif

#ifndef INT8_MAX
# define INT8_MAX 127
#endif
#ifndef INT8_MIN
# define INT8_MIN -128
#endif
#ifndef UINT8_MAX
# define UINT8_MAX 255
#endif
#ifndef INT16_MAX
# define INT16_MAX 32767
#endif
#ifndef INT16_MIN
# define INT16_MIN -32768
#endif
#ifndef UINT16_MAX
# define UINT16_MAX 65535
#endif
#ifndef INT32_MAX
# define INT32_MAX 2147483647
#endif
#ifndef INT32_MIN
/* MSC treats -2147483648 as -(2147483648U). */
# define INT32_MIN (-INT32_MAX - 1)
#endif
#ifndef UINT32_MAX
# define UINT32_MAX 4294967295U
#endif

#endif /* _MSC_VER */


#if !defined(INT64_MAX)
# define INT64_MAX 9223372036854775807LL
#endif
#if !defined(INT64_MIN)
# define INT64_MIN ((-INT64_MAX)-1)
#endif
#if !defined(UINT64_MAX)
# define UINT64_MAX 0xffffffffffffffffULL
#endif

#endif  /* SUPPORT_DATATYPES_H */

