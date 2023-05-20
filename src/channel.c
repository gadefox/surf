/* See LICENSE file for copyright and license details. */

#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>

#include "channel.h"
#include "verbose.h"
#include "surf.h"


static int sock = -1;


void
channel_socket_free (void)
{
    close (sock);
    sock = -1;
}

void
channel_init (int socket, GIOFunc iofunc)
{
    GIOChannel *gchansock;
    GIOFlags flags;
    
    sock = socket;

    gchansock = g_io_channel_unix_new (sock);
    g_io_channel_set_encoding (gchansock, NULL, NULL);

    flags = g_io_channel_get_flags (gchansock);
    g_io_channel_set_flags (gchansock, flags | G_IO_FLAG_NONBLOCK, NULL);

    g_io_channel_set_close_on_unref (gchansock, TRUE);
    g_io_add_watch (gchansock, G_IO_IN, iofunc, NULL);
}

void
channel_send (const char *msg, uint len)
{
    if ( send (sock, msg, len, 0) == (ssize_t)len )
        info ("socket: sent %d bytes", len);
    else
        error ("socket: sending");
}

static gboolean
channel_receive (GIOChannel *channel, char *buffer, uint size)
{
    GError *gerror = NULL;
    gsize read;

    /* read the message size: 3 characters */
    if ( g_io_channel_read_chars (channel, buffer, size, &read, &gerror) != G_IO_STATUS_NORMAL ) {
        /* this fn will destroy gerror object */
        verbose_gerror (gerror, "socket: reading");
        return FALSE;
    }

    /* check $read  */
    if ( read < size ) {
        error ("socket: message too short");
        return FALSE;
    }

    return TRUE;
}

gboolean
channel_read (GIOChannel *channel, ChannelMessage *chmsg)
{
    char header [CHANNEL_MESSAGE_HEADER_SIZE];
    char *cur, *msg;
    uint size;

    /* read the message header */
    if ( !channel_receive (channel, header, CHANNEL_MESSAGE_HEADER_SIZE ) )
        return FALSE;

    /* deserialize $pageid */
    cur = encoding_deserialize_uint64 (header, &chmsg->pageid);

    /* deserialize $token */
    cur = encoding_deserialize_uint_char2 (cur, &chmsg->token);

    /* deserialize message size */
    /* cur = */encoding_deserialize_uint_char3 (cur, &size);

#if DUMP
    verbose_uint64 ("pageid", chmsg->pageid);
    verbose_uint ("token", chmsg->token);
    verbose_uint ("body size", size);
#endif  /* DUMP */

    /* create new buffer and read the rest of the messahe */
    msg = malloc (size * sizeof (char));
    if ( msg == NULL ) {
        error_memory ();
        return FALSE;
    }

    /* read the body */
    if ( !channel_receive (channel, msg, size) ) {
        free (msg);
        return FALSE;
    }

    /* return the message */
    chmsg->msg = msg;
    return TRUE;
}

/* helpers for the serialization */
char *
channel_serialize_new (uint64 page_id, ChannelToken token, uint body_size, uint *msg_size, char **cur_buf)
{
    char *buffer, *cur;
    uint ret_size;

    /* allocate buffer */
    ret_size = body_size + CHANNEL_MESSAGE_HEADER_SIZE;
    buffer = malloc (ret_size);
    if ( buffer == NULL )
        return NULL;

    /* serialize $page_id: ENCODING_UINT64_SIZE characters */
    cur = encoding_serialize_uint64 (buffer, page_id);

    /* serialize $token: 2 characters */
    cur = encoding_serialize_uint_char2 (cur, token);

    /* serialize body size: 3 characters */
    cur = encoding_serialize_uint_char3 (cur, body_size);

    *msg_size = ret_size;
    *cur_buf = cur;
    return buffer;
}
