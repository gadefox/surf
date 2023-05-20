/* See LICENSE file for copyright and license details. */

#ifndef _SERIALIZE_H_
#define _SERIALIZE_H_

#include "channel.h"
#include "jsonrc.h"


char * serialize_user_password_msg (uint64 pageid, UserPasswordParameters *upparams, uint *size);

#ifdef FEATURE_SCROLL
char * serialize_scroll_msg (uint64 pageid, gboolean vert, int value, uint *size);
#endif


#endif /* _SERIALIZE_H_ */
