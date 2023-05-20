/* See LICENSE file for copyright and license details. */

#ifndef _WEB_KIT_H_
#define _WEB_KIT_H_

#include <webkit2/webkit2.h>
#include <X11/Xlib.h>

#include "callback.h"


#define webkit_free()  (webkit_socket_free ())


extern const char *wk_prefix_http;
extern const char *wk_prefix_https;
extern const char *wk_prefix_about;
extern const char *wk_prefix_file;
extern const char *wk_prefix_data;
extern const char *wk_prefix_blob;

extern Window wk_embed;
extern gboolean wk_show_xid;
extern int wk_sock_in;


void webkit_init (void);
void webkit_socket_free (void);

gboolean webkit_read_message (GIOChannel *channel, GIOCondition condition, gpointer unused);
void webkit_execute_run_command (Client *c, CommandType type, RunCommandParameters *params);

void webkit_decide_navigation (WebKitPolicyDecision *d, Client *c);
void webkit_decide_new_window (WebKitPolicyDecision *d, Client *c);
bool webkit_uri_is_ascii (const gchar *uri);
void webkit_decide_resource (WebKitPolicyDecision *d, Client *c);

WebKitCookieAcceptPolicy webkit_cookie_policy_get (int cookie_policy);
char webkit_cookie_policy_to_char (const WebKitCookieAcceptPolicy p);
const char * webkit_cookie_policy_to_string (char c);

void webkit_handle_go (Client *c, GoParameters *gp);
void webkit_handle_find_in_page (WebKitFindController *finder, FindInPageParameters *fp);
void webkit_handle_user_password (Client *c, UserPasswordParameters* up);

GdkFilterReturn webkit_handle_xevents (XEvent *xevent, GdkEvent *event, Client *c);

#ifdef FEATURE_TITLE
gboolean webkit_event_notify_enter (GtkWidget *w, GdkEvent *e, Client *c);
gboolean webkit_event_notify_leave (GtkWidget *w, GdkEvent *e, Client *c);
#endif  /* FEATURE_TITLE */

gboolean webkit_event_win_state (GtkWidget *w, GdkEvent *e, Client *c);
gboolean webkit_event_key_press (GtkWidget *w, GdkEvent *e, Client *c);

void webkit_window_create (Client *c);
void webkit_window_destroy (GtkWidget *w, Client *c);


#endif /* _WEB_KIT_H_ */
