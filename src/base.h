/* See LICENSE file for copyright and license details. */

#ifndef _BASE_H_
#define _BASE_H_


#define countof(a)       (sizeof (a) / sizeof ((a) [0]))
#define SETBIT(b, f, c)  { if (c) (b) |= (f); else (b) &= ~(f); }

#ifndef TRUE
#define TRUE   1
#endif

#ifndef FALSE
#define FALSE  0
#endif

#define INT_BITS   (sizeof (int) << 3)
#define UINT_BITS  INT_BITS

#if defined __x86_64__ && !defined __ILP32__
  #define INT_MAX_SIZE     (countof ("-9223372036854775807"))
  #define UINT_MAX_SIZE    (countof ("18446744073709551615"))
  #define UINT_HEX_SIZE    19  /* 64 (=bits) / 8 (=bytes) * 2 (=chars) + 2 (="0x") + 1 (='\0') */

  #define INT64_MAX_SIZE   INT_MAX_SIZE
  #define UINT64_MAX_SIZE  UINT_MAX_SIZE
  #define UINT64_HEX_SIZE  UINT_HEX_SIZE
#else
  #define INT_MAX_SIZE     (countof ("-2147483647"))
  #define UINT_MAX_SIZE    (countof ("4294967295"))
  #define UINT_HEX_SIZE    11  /* 32 (=bits) / 8 (=bytes) * 2 (=chars) + 2 (="0x") + 1 (='\0') */

  #define INT64_MAX_SIZE   (countof ("-9223372036854775807"))
  #define UINT64_MAX_SIZE  (countof ("18446744073709551615"))
  #define UINT64_HEX_SIZE  17
#endif


typedef unsigned char  byte;
typedef unsigned int   uint;
typedef unsigned long  ulong;

typedef unsigned long long  uint64;
typedef long long  int64;


#endif  /* _BASE_H_ */
