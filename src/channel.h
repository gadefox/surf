/* See LICENSE file for copyright and license details. */

#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include <glib.h>

#include "base.h"
#include "encoding.h"


#define CHANNEL_MESSAGE_HEADER_SIZE  (ENCODING_UINT64_SIZE + 5)  /* message token(2) + size(3) */


typedef enum {
    ChannelUndefined,
    ChannelTokenScroll,
    ChannelTokenUserPassword
} ChannelToken;


typedef struct {
    uint64 pageid;
    ChannelToken token;
    char *msg;
} ChannelMessage;


void channel_init (int socket, GIOFunc iofunc);
void channel_socket_free (void);

/* sending message */
void channel_send (const char *msg, uint len);

/* receiving message */
gboolean channel_read (GIOChannel *channel, ChannelMessage *chmsg);

/* serialize */
char * channel_serialize_new (uint64 page_id, ChannelToken token, uint body_size, uint *msg_size, char **cur_buf);


#endif  /* _CHANNEL_H_ */
