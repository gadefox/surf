/* See LICENSE file for copyright and license details. */

#include <stdio.h>

#include "string.h"
#include "verbose.h"


gboolean
s_is_size (String *s, uint add)
{
    uint new_size, new_len, len;
    char *new_head;

    /* we want to append $add characters */
    len = s->tail - s->head;
    new_len = len + add;

    /* the actual size has to be greater then the new length due to '\0' character (equal is not enough) */
    if ( new_len < s->size )
        return TRUE;
   
    new_size = s->size << 1;
    if ( new_size <= new_len )
        new_size = new_len + 1;  /* plus '\0' */

    /* c90: realloc with null pointer must work equivalently to malloc! */
    new_head = realloc (s->head, new_size * sizeof (char));
    if ( new_head == NULL ) {
        /* verbose an error only once */
        if ( s->flags & StringOutOfMemory ) {
            error_memory ();

            /* don't spam with out of memory messages */
            s->flags &= ~StringOutOfMemory;
        }

        /* set an error flag */
        s->flags |= StringError;

        /* leaking memory: although realloc has failed the $s->head memory block is untouched, so we can't set the pointer to null! */
        return FALSE;
    }

    /* update structure members */
    s->head = new_head;
    s->tail = new_head + len;
    s->size = new_size;
    return TRUE;
}

void s_add_char (String *s, char c)
{
    if ( s_is_size (s, 1) )
        *(s->tail)++ = c;
}

void s_add_str (String *s, const char *val, int len)
{
    if ( len == -1 )
        len = strlen (val);

    if ( s_is_size (s, len) ) {
        memcpy (s->tail, val, len);
        s->tail += len;
    }
}

void s_add_int (String *s, int val)
{
    if ( s_is_size (s, INT_MAX_SIZE - 1) )
        s->tail = s_int (s->tail, val);
}

void s_add_uint (String *s, uint val)
{
    if ( s_is_size (s, UINT_MAX_SIZE - 1) )
        s->tail = s_uint (s->tail, val);
}

void s_add_uint64 (String *s, uint64 val)
{
    if ( s_is_size (s, UINT64_MAX_SIZE - 1) )
        s->tail = s_uint64 (s->tail, val);
}

void s_add_int64 (String *s, int64 val)
{
    if ( s_is_size (s, INT64_MAX_SIZE - 1) )
        s->tail = s_int64 (s->tail, val);
}

gboolean
s_close (String *s)
{
    if ( s->flags & StringError )
        return FALSE;

    *(s->tail) = '\0';
    return TRUE;
}

void
s_free (String *s)
{
    if ( s->flags & StringOwnsBuffer )
        free (s->head);
}

void
s_init (String *s, uint size)
{
    char *new_buf;

    /* initialize the flags */
    s->flags = StringOwnsBuffer | StringOutOfMemory;

    /* create buffer */
    new_buf = malloc (size * sizeof (char));
    if ( new_buf == NULL ) {
        /* print the out of memory error message. */
        error_memory ();

        /* no StringOutOfMemory: we don't want to spam with out of memory errors messages.  Note we didn't set the error flags because maybe realloc will be successfull. */
        s->flags &= ~StringOutOfMemory;

        /* the malloc has failed so the buffer size is zero. */
        size = 0;
    }

    /* initialize structure members */
    s->head = new_buf;
    s->tail = new_buf;
    s->size = size;
}


/* helpers */
char *
s_set_zero (char *s)
{
    *s++ = '0';
    *s = '\0';
    return s;
}

/* e points to s + strlen (s) ~ *e should be '\0' */
void
s_reverse_end (char *s, char *e)
{
    char c;

    while ( s < --e ) {
        /* swap characters */
        c = *s;
        *s++ = *e;
        *e = c;
    }
}

void
s_reverse (char *s)
{
    uint len;

    len = strlen (s);
    s_reverse_end (s, s + len);
}

char *
s_uint (char *s, uint val)
{
    char *e;

    if ( val == 0 )
        return s_set_zero (s);

    /* process individual digits */
    e = s;
    while ( val != 0 ) {
        *e++ = '0' + val % 10;
        val /= 10;
    }
    s_reverse_end (s, e);

    *e = '\0';
    return e;
}

char *
s_int (char *s, int val)
{
    if ( val < 0 ) {
        *s++ = '-';
        val = -val;
    }
    return s_uint (s, val);
}

char *
s_uint64 (char *s, uint64 val)
{
    char *e;

    if ( val == 0 )
        return s_set_zero (s);

    /* process individual digits */
    e = s;
    while ( val != 0 ) {
        *e++ = '0' + val % 10;
        val /= 10;
    }
    s_reverse_end (s, e);

    *e = '\0';
    return e;
}

char *
s_int64 (char *s, int64 val)
{
    if ( val < 0 ) {
        *s++ = '-';
        val = -val;
    }
    return s_uint64 (s, val);
}

char *
s_hex (char *s, uint val)
{
    int mod;
    char *e;

    *s++ = '0';
    *s++ = 'x';

    if ( val == 0 )
        return s_set_zero (s);

    /* process individual digits */
    e = s;
    while ( val != 0 ) {
        mod = val % 16;
        *e++ = mod < 10 ? mod + '0' : mod - 10 + 'a';
        val /= 16;
    }
    s_reverse_end (s, e);

    *e = '\0';
    return e;
}
