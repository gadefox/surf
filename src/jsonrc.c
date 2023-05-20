/* See LICENSE file for copyright and license details. */

#include <stdio.h>

#include "jsonrc.h"
#include "verbose.h"
#include "surf.h"
#include "file.h"
#include "client.h"


/* CommandType */
static const char rc_name [] = "rc";

/* CommandFindInPage */
static const char keyword_name [] = "keyword";
static const char options_name [] = "options";

/* CommandUserPassword */
static const char user_name [] = "user";
static const char password_name [] = "password";

/* run command info */
static const char uri_name [] = "uri";
static const char user_agent_name [] = "useragent";
static const char cookies_name [] = "cookies";


gchar *
json_build_webpage_info (const char *uri)
{
    JsonBuilder *builder;
    JsonGenerator *json_gen;
    JsonNode *root;
    gchar *ret;
  
    /* build json structure */ 
    builder = json_builder_new ();
    json_builder_begin_object (builder);

    /* uri field */
    json_builder_set_member_name (builder, uri_name);
    json_builder_add_string_value (builder, uri);

    /* user agent */
    json_builder_set_member_name (builder, user_agent_name);
    json_builder_add_string_value (builder, user_agent);

    /* cookie file */
    json_builder_set_member_name (builder, cookies_name);
    json_builder_add_string_value (builder, file_cookie);

    /* root */
    json_builder_end_object (builder);
    root = json_builder_get_root (builder);

    /* generate json string */
    json_gen = json_generator_new ();
    json_generator_set_root (json_gen, root);
    ret = json_generator_to_data (json_gen, NULL);

    /* free allocated objects */
    json_node_free (root);
    g_object_unref (json_gen);
    g_object_unref (builder);

    return ret;
}

JsonParser *
json_parser_load (const char *jsonrc)
{
    JsonParser *parser;
    GError *gerror = NULL;
 
#ifdef DUMP    
    /* verbose */
    info ("json: %s", jsonrc);
#endif  /* DUMP */

    /* initialize json parser */
    parser = json_parser_new ();

    /* parse run command */
    if ( !json_parser_load_from_data (parser, jsonrc, -1, &gerror) ) {
        /* this fn will destroy gerror object */
        verbose_gerror (gerror, "unable to parse json");

        /* destroy json parser */
        g_object_unref (parser);
        return NULL;
    }

    /* success means the gerror variable should be null */
    return parser;
}

CommandType
json_parse_run_command (const char *jsonrc, RunCommandParameters *rcparams)
{
    JsonParser *parser;
    JsonReader *reader;
    JsonNode *root;
    CommandType rctype;
   
    /* initialize json parser and parse run command */
    parser = json_parser_load (jsonrc);
    if ( parser == NULL )
        return CommandUndefined;
    
    root = json_parser_get_root (parser);
    reader = json_reader_new (root);
    rctype = json_read_run_command (reader, rcparams);
                        
    /* destroy allocated object */
    g_object_unref (reader);
    g_object_unref (parser);

    return rctype;
}

CommandType
json_read_find_in_page_params (JsonReader *reader, FindInPageParameters *fp)
{
    /* read $keyword (string) */
    fp->keyword = json_read_string (reader, keyword_name);
    if ( fp->keyword == NULL )
        return CommandUndefined;

    /* read $value (string) and $options (int) */
    fp->options = json_read_int (reader, options_name);
    return CommandFindInPage;
}

CommandType
json_read_user_password_params (JsonReader *reader, UserPasswordParameters *up)
{
    /* read $user (string) */
    up->user = json_read_string (reader, user_name);
    if ( up->user == NULL )
        return CommandUndefined;

    /* read $password (string) */
    up->password = json_read_string (reader, password_name);
    if ( up->password == NULL ) {
        g_free (up->user);
        return CommandUndefined;
    }
    return CommandUserPassword;
}

CommandType
json_read_go_params (JsonReader *reader, GoParameters *gp)
{
    /* read $uri (string) */
    gp->uri = json_read_string (reader, uri_name);
    if ( gp->uri == NULL )
        return CommandUndefined;

    return CommandGo;
}

CommandType
json_read_run_command (JsonReader *reader, RunCommandParameters *rcparams)
{
    CommandType rctype;

    /* read $run command type */
    rctype = json_read_int (reader, rc_name);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
    switch ( rctype )
      {
        case CommandFindInPage:
            return json_read_find_in_page_params (reader, (FindInPageParameters *) rcparams);

        case CommandUserPassword:
            return json_read_user_password_params (reader, (UserPasswordParameters *) rcparams);

        case CommandGo:
            return json_read_go_params (reader, (GoParameters *) rcparams);
      }
#pragma GCC diagnostic pop

    return CommandUndefined;
}


/* functions for Command opeations */
void
find_in_page_params_free (FindInPageParameters *fp)
{
    free (fp->keyword);
}

void
user_password_params_free (UserPasswordParameters *up)
{
    free (up->user);
    free (up->password);
}

void
go_params_free (GoParameters *gp)
{
    free (gp->uri);
}

void
run_command_params_free (CommandType type, RunCommandParameters *params)
{
    /* free the content */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
    switch ( type )
      {
        case CommandFindInPage:
            find_in_page_params_free ((FindInPageParameters *) params);
            break;

        case CommandUserPassword:
            user_password_params_free ((UserPasswordParameters *) params);
            break;

        case CommandGo:
            go_params_free ((GoParameters *) params);
            break;
      }
#pragma GCC diagnostic pop    
}


/* json helpers */

gchar *
json_read_string (JsonReader *reader, const char *name)
{
    const char *val;

    /* string field */
    json_reader_read_member (reader, name);
    val = json_reader_get_string_value (reader);
    json_reader_end_member (reader);

#ifdef DUMP
    verbose_string (name, val);
#endif  /* DUMP */

    /* duplicate string */
    return g_strdup (val);
}

int
json_read_int (JsonReader *reader, const char *name)
{
    int val;

    /* integer field */
    json_reader_read_member (reader, name);
    val = json_reader_get_int_value (reader);
    json_reader_end_element (reader);

#ifdef DUMP
    verbose_int (name, val);
#endif  /* DUMP */

    return val;
}
