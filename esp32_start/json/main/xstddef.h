/**
 * @brief Basic definition
 * @file xstddef.h
 * @version 1.0
 * @author zhangxc
 * @date 12/1/2015 14:6:49
 * @par Copyright:
 * Copyright (c) Leedarson Lighting 2000-2015. All rights reserved.
 *
 * @history
 * 1.Date        : 12/1/2015 14:6:49
 *   Author      : zhangxc
 *   Modification: Created file
 */


#ifndef __XSTDDEF_H__
#define __XSTDDEF_H__

#ifdef __cplusplus
extern "C" {
#endif  // End for __cplusplus

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  XFALSE = 0, XTRUE = !XFALSE
}
bool_t;

typedef char xt_s8;
typedef unsigned char xt_u8;
typedef signed short xt_s16;
typedef unsigned short xt_u16;
typedef signed int xt_s32;
typedef unsigned int xt_u32;
typedef long long xt_s64;
typedef unsigned long long xt_u64;
typedef char *xt_string;
typedef const char *xt_rostring;
typedef unsigned char xt_bool;
#ifdef __GCC__
typedef void xt_void;
#else
#define xt_void void
#endif
typedef const void xt_rovoid;
typedef float xt_float;
typedef double xt_double;

#ifndef NULL
#define NULL ((void *)0)
#endif

enum xt_result_e {
  XERROR_EMPTY = -6,
  XERROR_TIMEOUT = -5,
  XERROR_MEM = -4,
  XERROR_EXIST = -3,
  XERROR_FUNC_ARGS = -2,
  XERROR = -1,
  XOK = 0
};
typedef enum xt_result_e xt_result;
typedef enum xt_result_e xt_ret;

struct Xtm_t {
  int tm_sec;         /* seconds */
  int tm_min;         /* minutes */
  int tm_hour;        /* hours */
  int tm_mday;        /* day of the month */
  int tm_mon;         /* month */
  int tm_year;        /* year */
  int tm_wday;        /* day of the week */
  int tm_yday;        /* day in the year */
  int tm_isdst;       /* daylight saving time */
};
typedef struct Xtm_t Xtm;

#if defined (__ICCARM__)
#define STR(s) #s
#define SECTION(_name)      _Pragma(STRINGIFY(location = _name))
#define ALIGNMTO(_bound)    _Pragma(STRINGIFY(data_alignment = ##_bound##))
#define PACKED_             __packed
#define LONG_CALL_
#define LONG_CALL_ROM_
#define WEAK                __weak
#define INLINE              inline
#else
#define SECTION(_name)      __attribute__ ((__section__(_name)))
#define ALIGNMTO(_bound)    __attribute__ ((aligned (_bound)))
#define PACKED_             __attribute__ ((packed))
#define LONG_CALL_          __attribute__ ((long_call))
#define WEAK                __attribute__ ((weak))
#define INLINE              __inline
#endif

#define IN
#define OUT
#define INOUT


#ifdef __cplusplus
}
#endif  // End for __cplusplus

#endif  // End for __XSTDDEF_H__

