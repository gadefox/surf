/* See LICENSE file for copyright and license details. */

#ifndef _VERBOSE_H_
#define _VERBOSE_H_

#include <stdarg.h>
#include <glib.h>

#include "base.h"


#define verbose_s(s)       (fputs ((s), stderr))
#define verbose_c(c)       (fputc ((c), stderr))
#define verbose_newline()  (verbose_c ('\n'))
#define verbose_comma()    { verbose_c (','); verbose_c (' '); }


typedef enum {
    VerboseBlack = 30,
    VerboseRed = 31,
    VerboseGreen = 32,
    VerboseYellow = 33,
    VerboseBlue = 34,
    VerboseMagenda = 35,
    VerboseCyan = 36,
    VerboseWhite = 37
} VerboseColor;


extern const char *prog_name;


void verbose_color_begin (VerboseColor color);
void verbose_color_end (void);
void verbose_color (const char *str, VerboseColor color);
void verbose_prefix (const char *prefix);
void verbose_color_prefix (const char *prefix, VerboseColor color);
void verbose (const char *prefix, VerboseColor color, const char *err, va_list params);

void verbose_gerror (GError *gerror, const char *msg);

void verbose_v (const char **v);
void verbose_i (int val);
void verbose_u (uint val);
void verbose_spaces (uint count);

void verbose_boolean (const char *name, gboolean val);
void verbose_string (const char *name, const char *val);
void verbose_int (const char *name, int val);
void verbose_uint (const char *name, uint val);
void verbose_u64 (uint64 val);
void verbose_uint64 (const char *name, uint64 val);
void verbose_i64 (int64 val);
void verbose_int64 (const char *name, int64 val);

void info (const char *format, ...);
void warn (const char *format, ...);
void error (const char *format, ...);
void error_memory (void);


#endif /* _VERBOSE_H_ */
