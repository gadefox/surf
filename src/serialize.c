/* See LICENSE file for copyright and license details. */

#include "serialize.h"


char *
serialize_user_password_msg (uint64 pageid, UserPasswordParameters *upparams, uint *size)
{
    uint user_len, password_len;
    char *msg, *cur;

    /* allocate the msg and initialize it's header */
    user_len = strlen (upparams->user);
    password_len = strlen (upparams->password);

    msg = channel_serialize_new (pageid, ChannelTokenUserPassword,
              ENCODING_STRING_SIZE (user_len) + ENCODING_STRING_SIZE (password_len), size, &cur);
    if ( msg == NULL )
        return NULL;

    /* serialize $user (string) and $password (string) */
    cur = encoding_serialize_string (cur, upparams->user, user_len);
    /*cur = */encoding_serialize_string (cur, upparams->password, password_len);

    return msg;
}

#ifdef FEATURE_SCROLL

char *
serialize_scroll_msg (uint64 pageid, gboolean vert, int value, uint *size)
{
    char *msg, *cur;

    /* allocate the msg and initialize it's header */
    msg = channel_serialize_new (pageid, ChannelTokenScroll, 3, size, &cur);  /* boolean(1), value(2) */
    if ( msg == NULL )
        return NULL;

    /* serialize $vert: 1 character */
    *cur++ = encoding_serialize_boolean (vert);

    /* serialize $content: content_len + 2 characters */
    /*cur = */encoding_serialize_int_char2 (cur, value);

    return msg;
}

#endif  /* FEATURE_SCROLL */
