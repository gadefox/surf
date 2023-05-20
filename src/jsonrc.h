/* See LICENSE file for copyright and license details. */

#ifndef _JSON_RC_H_
#define _JSON_RC_H_

#include <json-glib/json-glib.h>

#include "arg.h"


typedef struct {
    gchar *keyword;
    uint options;
} FindInPageParameters;

typedef struct {
    gchar *user;
    gchar *password;
} UserPasswordParameters;

typedef struct {
    gchar *uri;
} GoParameters;

typedef union {
    GoParameters gp;
    FindInPageParameters fp;
    UserPasswordParameters up;
} RunCommandParameters;


/* json building */
gchar * json_build_webpage_info (const char *uri);

/* json reading */
JsonParser * json_parser_load (const char *jsonrc);

CommandType json_parse_run_command (const char *jsonrc, RunCommandParameters *rcparams);
CommandType json_read_run_command (JsonReader *reader, RunCommandParameters *rcparams);
CommandType json_read_find_in_page_params (JsonReader *reader, FindInPageParameters *fp);
CommandType json_read_user_password_params (JsonReader *reader, UserPasswordParameters *up);
CommandType json_read_go_params (JsonReader *reader, GoParameters *gp);

/* helpers */
gchar * json_read_string (JsonReader *reader, const char *name);
int json_read_int (JsonReader *reader, const char *name);

/* run command structure */
void run_command_params_free (CommandType type, RunCommandParameters *params);
void user_password_params_free (UserPasswordParameters *up);
void find_in_page_params_free (FindInPageParameters *fp);
void go_params_free (GoParameters *gp);


#endif /* _JSON_RC_H_ */
