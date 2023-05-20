/* See LICENSE file for copyright and license details. */

#ifndef _ENCODING_H_
#define _ENCODING_H_

#include "base.h"


/* This encoding has been implemented because I realized I had to send
 * characters instead of bytes.  I don't know the background but the Linux
 * is full of weird surprices. */
#define ENCODING_BASE  6

#define UINT_CHAR_MAX   ((1 << (ENCODING_BASE * 1)) - 1)       /* base6: 63 =     0x3F */
#define UINT_CHAR2_MAX  ((1 << (ENCODING_BASE * 2)) - 1)       /* base6: 4095 =   0xFFF */
#define UINT_CHAR3_MAX  ((1 << (ENCODING_BASE * 3)) - 1)       /* base6: 262143 = 0x3FFFF */
#define UINT_CHAR4_MAX  ((1 << (ENCODING_BASE * 4)) - 1)       /* base6: TODO */
#define UINT_CHAR5_MAX  ((1 << (ENCODING_BASE * 5)) - 1)       /* base6: TODO */

#define INT_CHAR_MAX    ((1 << (ENCODING_BASE * 1 - 1)) - 1)   /* base6: 31 =     0x1F */
#define INT_CHAR2_MAX   ((1 << (ENCODING_BASE * 2 - 1)) - 1)   /* base6: 2047 =   0x7FF */
#define INT_CHAR3_MAX   ((1 << (ENCODING_BASE * 3 - 1)) - 1)   /* base6: 131071 = 0x1FFFF */
#define INT_CHAR4_MAX   ((1 << (ENCODING_BASE * 4 - 1)) - 1)   /* base6: TODO */
#define INT_CHAR5_MAX   ((1 << (ENCODING_BASE * 5 - 1)) - 1)   /* base6: TODO */

/* single character uint uint64 */
#define encoding_serialize_uint_char1(v)        ((((uint) (v)) & UINT_CHAR_MAX) + 48)  /* '0' */
#define encoding_deserialize_uint_char1(c)      ((c) - 48UL)   /* '0' */

#define encoding_serialize_uint64_char1         define encoding_serialize_uint_char1
#define encoding_deserialize_uint64_char1     define encoding_serialize_uint_char1

/* boolean */
#define encoding_serialize_boolean(v)          ((v) ? 'A' : 'B')
#define encoding_deserialize_boolean(c)        (encoding_deserialize_uint_char1 (c))

/* internal macros */
#define encoding_serialize_uint_shift(v, b)        (encoding_serialize_uint_char1 ((v) >> ((b) * ENCODING_BASE)))

#define ENCODING_INT_SIGN                          (1 << (ENCODING_BASE - 1))

#define encoding_serialize_int_char(v)             ((((uint) (v)) & INT_CHAR_MAX) + 48)  /* '0' */
#define encoding_serialize_int_shift(v, b)         (encoding_serialize_int_char    ((v) >> ((b) * ENCODING_BASE)))
#define encoding_serialize_int_shift_sign(v, b)    (encoding_serialize_uint_char1 (((v) >> ((b) * ENCODING_BASE)) | ENCODING_INT_SIGN))

#define encoding_deserialize_uint_shift(c, b)      (encoding_deserialize_uint_char1   (c) << ((b) * ENCODING_BASE))
#define encoding_deserialize_uint64_shift(c, b)    (encoding_deserialize_uint64_char1 (c) << ((b) * ENCODING_BASE))
#define encoding_deserialize_int_shift(v, b)       ((v) << ((b) * ENCODING_BASE))
#define encoding_deserialize_int_shift_sign(v, b)  (((v) & ~ENCODING_INT_SIGN) << ((b) * ENCODING_BASE))

/* uint uint64 */
#define ENCODING_SIZE(b)      (((b) % ENCODING_BASE) == 0 ? (b) / ENCODING_BASE : (b) / ENCODING_BASE + 1)
#define ENCODING_UINT_SIZE    (ENCODING_SIZE (UINT_BITS))
#define ENCODING_UINT64_SIZE  (ENCODING_SIZE (64))

#define encoding_serialize_uint(b, v)    (encoding_serialize_uint_iter   (ENCODING_UINT_SIZE, (b), (v)))
#define encoding_deserialize_uint(b, v)  (encoding_deserialize_uint_iter (ENCODING_UINT_SIZE, (b), (v)))

/* int */
#define ENCODING_INT_SIZE    ENCODING_UINT_SIZE

#define encoding_serialize_int(b, v)    (encoding_serialize_int_iter   (ENCODING_UINT_SIZE - 1, (b), (v)))
#define encoding_deserialize_int(b, v)  (encoding_deserialize_int_iter (ENCODING_UINT_SIZE - 1, (b), (v)))


/* We don't want to bloat everything incl. the memory usage, right??
 * So these fn(s) are usefull when we don't want to encode the whole uint
 * (32 bits needs 6 characters) but the 6 (12) bits are not enough e.g.
 * Let's say we want to serialize unsigned short (16 bits ~ max value is
 * 65535). This requires 3 characters (18 bits) for the serialization, so
 * 2 bits are not used (wasted). Using the remaining 2 bits we are able to
 * serialize greater value (up to 262143) without extra memory.
 * This principle is used for the string serialization. I'm pretty sure
 * 4095 characters is enough for the strings so we need to allocate 2
 * characters only instead of 6 (uint32). */
#define encoding_serialize_uint_char2(b, v)    (encoding_serialize_uint_iter   (2, (b), (v)))
#define encoding_serialize_uint_char3(b, v)    (encoding_serialize_uint_iter   (3, (b), (v)))
#define encoding_serialize_uint_char4(b, v)    (encoding_serialize_uint_iter   (4, (b), (v)))
#define encoding_serialize_uint_char5(b, v)    (encoding_serialize_uint_iter   (5, (b), (v)))

#define encoding_deserialize_uint_char2(b, v)  (encoding_deserialize_uint_iter (2, (b), (v)))
#define encoding_deserialize_uint_char3(b, v)  (encoding_deserialize_uint_iter (3, (b), (v)))
#define encoding_deserialize_uint_char4(b, v)  (encoding_deserialize_uint_iter (4, (b), (v)))
#define encoding_deserialize_uint_char5(b, v)  (encoding_deserialize_uint_iter (5, (b), (v)))

#define encoding_serialize_int_char1(b, v)      (encoding_serialize_int_iter   (0, (b), (v)))
#define encoding_serialize_int_char2(b, v)     (encoding_serialize_int_iter    (1, (b), (v)))
#define encoding_serialize_int_char3(b, v)     (encoding_serialize_int_iter    (2, (b), (v)))
#define encoding_serialize_int_char4(b, v)     (encoding_serialize_int_iter    (3, (b), (v)))
#define encoding_serialize_int_char5(b, v)     (encoding_serialize_int_iter    (4, (b), (v)))

#define encoding_deserialize_int_char1(b, v)    (encoding_deserialize_int_iter (0, (b), (v)))
#define encoding_deserialize_int_char2(b, v)   (encoding_deserialize_int_iter  (1, (b), (v)))
#define encoding_deserialize_int_char3(b, v)   (encoding_deserialize_int_iter  (2, (b), (v)))
#define encoding_deserialize_int_char4(b, v)   (encoding_deserialize_int_iter  (3, (b), (v)))
#define encoding_deserialize_int_char5(b, v)   (encoding_deserialize_int_iter  (4, (b), (v)))

/* uint64 */
char * encoding_serialize_uint64 (char *buf, uint64 val);
char * encoding_deserialize_uint64 (const char *buf, uint64 *ret);

/* uint */
char * encoding_serialize_uint_iter (uint iter, char *buf, uint val);
char * encoding_deserialize_uint_iter (uint iter, const char *buf, uint *ret);

/* int */
char * encoding_serialize_int_iter (uint iter, char *buf, int val);  /* only for neg values */
char * encoding_deserialize_int_iter (uint iter, const char *buf, int *ret);

/* string */
#define ENCODING_STRING_SIZE(l)  ((l) + 2)  /* UINT_CHAR2 */

char * encoding_serialize_string (char *buf, const char *s, uint len);
char * encoding_deserialize_string (const char *buf, char **ret_s, uint *ret_len);


#endif  /* _ENCODING_H_ */
