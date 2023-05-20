/* See LICENSE file for copyright and license details. */

#include "verbose.h"
#include "surf.h"
#include "string.h"


const char *prog_name;


void
verbose_color_begin (VerboseColor color)
{
    fprintf (stderr, "\033[%d;1m", color);
}

void
verbose_color_end (void)
{
    verbose_s ("\033[0m");
}

void verbose_color (const char *str, VerboseColor color)
{
    verbose_color_begin (color);
    verbose_s (str);
    verbose_color_end ();
}

void
verbose_prefix (const char *prefix)
{
    /* "$prefix: " */
    verbose_s (prefix);
    verbose_c (':');
    verbose_c (' ');
}

void
verbose_color_prefix (const char *prefix, VerboseColor color)
{
    verbose_color_begin (color);
    verbose_prefix (prefix);
    verbose_color_end ();
}

void
verbose (const char *prefix, VerboseColor color, const char *format, va_list params)
{
    verbose_prefix (prog_name);
    verbose_color_prefix (prefix, color);
    vfprintf (stderr, format, params);
    verbose_newline ();
}

void
verbose_boolean (const char *name, gboolean val)
{
    if ( name != NULL )
        verbose_color_prefix (name, VerboseCyan);
    
    if ( val )
        verbose_color ("YES", VerboseGreen);
    else
        verbose_color ("NO", VerboseRed);

    verbose_newline ();
}

void
verbose_string (const char *name, const char *val)
{
    if ( name != NULL )
        verbose_color_prefix (name, VerboseCyan);

    verbose_s (val);
    verbose_newline ();
}

void
verbose_gerror (GError *gerror, const char *msg)
{
    error (msg);

    if ( gerror != NULL ) {
        verbose_s (gerror->message);
        g_error_free (gerror);
        verbose_newline ();
    }
}

void
verbose_v (const char **v)
{
    const char *s;
    
    for ( s = *v; s != NULL; s = *++v ) {
        verbose_s (s);
        verbose_newline ();
    }
}

void
verbose_i (int val)
{
    char szval [INT_MAX_SIZE];

    s_int (szval, val);
    verbose_s (szval);
}

void
verbose_int (const char *name, int val)
{
    verbose_color_prefix (name, VerboseCyan);
    verbose_i (val);
    verbose_newline ();
}

void
verbose_u (uint val)
{
    char szval [UINT_MAX_SIZE];

    s_uint (szval, val);
    verbose_s (szval);
}

void
verbose_uint (const char *name, uint val)
{
    verbose_color_prefix (name, VerboseCyan);
    verbose_u (val);
    verbose_newline ();
}

void
verbose_u64 (uint64 val)
{
    char szval [UINT64_MAX_SIZE];

    s_uint64 (szval, val);
    verbose_s (szval);
}

void
verbose_uint64 (const char *name, uint64 val)
{
    verbose_color_prefix (name, VerboseCyan);
    verbose_u64 (val);
    verbose_newline ();
}

void
verbose_i64 (int64 val)
{
    char szval [INT64_MAX_SIZE];

    s_int64 (szval, val);
    verbose_s (szval);
}

void
verbose_int64 (const char *name, int64 val)
{
    verbose_color_prefix (name, VerboseCyan);
    verbose_i64 (val);
    verbose_newline ();
}

void
verbose_spaces (uint count)
{
    while (count-- != 0 )
        verbose_c (' ');
}

void
warn (const char *format, ...)
{
    va_list params;

    va_start (params, format);
    verbose ("WARNING", VerboseYellow, format, params);
    va_end (params);
}

void
error (const char *format, ...)
{
    va_list params;

    va_start (params, format);
    verbose ("ERROR", VerboseRed, format, params);
    va_end (params);
}

void
info (const char *format, ...)
{
    va_list params;

    va_start (params, format);
    verbose ("INFO", VerboseGreen, format, params);
    va_end (params);
}

void
error_memory (void)
{
    error ("out of memory");
}
