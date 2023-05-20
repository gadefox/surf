/* See LICENSE file for copyright and license details. */

#ifndef _STRING_H_
#define _STRING_H_

#include <glib.h>

#include "base.h"


typedef enum {
    StringOwnsBuffer  = (1 << 0),
    StringError       = (1 << 1),
    StringOutOfMemory = (1 << 2)
} StringFlags;


typedef struct {
    char *head;
    char *tail;
    uint size;
    uint flags;
} String;


void s_init (String *s, uint size);
gboolean s_is_size (String *s, uint add);
gboolean s_close (String *s);
void s_free (String *s);

void s_add_char (String *s, char c);
void s_add_str (String *s, const char *val, int len);
void s_add_int (String *s, int val);
void s_add_uint (String *s, uint val);
void s_add_uint64 (String *s, uint64 val);
void s_add_int64 (String *s, int64 val);


/* helpers */
char * s_set_zero (char *s);
void s_reverse_end (char *s, char *e);
void s_reverse (char *s);

char * s_uint (char *s, uint val);
char * s_int (char *s, int val);
char * s_hex (char *s, uint val);
char * s_uint64 (char *s, uint64 val);
char * s_int64 (char *s, int64 val);


#endif  /* _STRING_H_ */
