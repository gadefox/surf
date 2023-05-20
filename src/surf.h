/* See LICENSE file for copyright and license details. */

#ifndef _SURF_H_
#define _SURF_H_

#include <glib.h>

#include "arg.h"
#include "parameter.h"


extern int surf_is_user_agent;
extern char *surf_full_user_agent;

extern char *surf_style_file;
extern char *surf_uri_default;
extern Parameter surf_config [ParameterLast];
extern char *surf_argv0;


void surf_cleanup (void);


#endif /* _SURF_H_ */
