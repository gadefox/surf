/* See LICENSE file for copyright and license details. */

#include <string.h>
#include <stdlib.h>

#include "encoding.h"
#include "verbose.h"


/* STRING */
char *
encoding_serialize_string (char *buf, const char *val, uint len)
{
    /* We can't copy the string using strcpy due to termination character ('\0') so we will serialize the length and then the string content.
     * Note the $len must be between <0, 4096) */
    buf = encoding_serialize_uint_char2 (buf, len);

    /* according to c99 standard: we can pass zero length! */
    memcpy (buf, val, len);
    return buf + len;
}

char *
encoding_deserialize_string (const char *buf, char **ret_s, uint *ret_len)
{
    uint len;
    char *new_s;

    /* string length */
    buf = encoding_deserialize_uint_char2 (buf, &len);

    /* allocate new buffer */
    new_s = malloc (len + 1);  /* +1 for the termination character */
    if ( new_s == NULL ) {
        error_memory ();
        return NULL;
    }

    /* according to c99 standard: we can pass zero length! */
    memcpy (new_s, buf, len);
    new_s [len] = '\0';

    *ret_s = new_s;
    if ( ret_len != NULL )
        *ret_len = len;
    return (char *) buf + len;
}


/* UINT */
char *
encoding_serialize_uint_iter (uint iter, char *buf, uint val)
{
    uint idx;

    *buf++ = encoding_serialize_uint_char1 (val);

    for ( idx = 1; idx < iter; idx++ )
        *buf++ = encoding_serialize_uint_shift (val, idx);
    
    return buf;
}

char *
encoding_deserialize_uint_iter (uint iter, const char *buf, uint *ret)
{
    uint idx, val;

    val = encoding_deserialize_uint_char1 (*buf++);

    for ( idx = 1; idx < iter; idx++ )
        val |= encoding_deserialize_uint_shift (*buf++, idx);

    *ret = val;
    return (char *) buf;
}


/* UINT64 */
char *
encoding_serialize_uint64 (char *buf, uint64 val)
{
    uint idx;

    *buf++ = encoding_serialize_uint_char1 (val);

    for ( idx = 1; idx < ENCODING_UINT64_SIZE; idx++ )
        *buf++ = encoding_serialize_uint_shift (val, idx);

    return buf;
}

char *
encoding_deserialize_uint64 (const char *buf, uint64 *ret)
{
    uint idx, val;

    val = encoding_deserialize_uint_char1 (*buf++);

    for ( idx = 1; idx < ENCODING_UINT64_SIZE; idx++ )
        val |= encoding_deserialize_uint_shift (*buf++, idx);

    *ret = val;
    return (char *) buf;
}

/* INT */
char *
encoding_serialize_int_iter (uint iter, char *buf, int val)
{
    uint idx;
    gboolean neg = FALSE;

    if ( val < 0 ) {
        val = -val;
        neg = TRUE;
    }
    *buf++ = encoding_serialize_uint_char1 (val);

    for ( idx = 1; idx < iter; idx++ )
        *buf++ = encoding_serialize_uint_shift (val, idx);

    *buf = neg ? encoding_serialize_int_shift_sign (val, idx) :
                 encoding_serialize_int_shift (val, idx);
    return buf + 1;
}

char *
encoding_deserialize_int_iter (uint iter, const char *buf, int *ret)
{
    uint idx, signval;
    int val;

    val = encoding_deserialize_uint_char1 (*buf++);

    for ( idx = 1; idx < iter; idx++ )
        val |= encoding_deserialize_uint_shift (*buf++, idx);
    
    /* is the value negative? */
    signval = encoding_deserialize_uint_char1 (*buf);
    if ( signval & ENCODING_INT_SIGN ) {
        /* yes, so clear the sign (first) bit */
        val |= encoding_deserialize_int_shift_sign (signval, idx);
        *ret = -val;
    } else {
        val |= encoding_deserialize_int_shift (signval, idx);
        *ret = val;
    }
    return (char *) buf + 1;
}
