/* See LICENSE file for copyright and license details. */

#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <webkit2/webkit2.h>
#include <gtk/gtk.h>
#include <X11/X.h>

#include "jsonrc.h"
#include "channel.h"


typedef enum {
    ClientFullscreen  = (1 << 0),
    ClientHttps       = (1 << 1),
    ClientInsecure    = (1 << 2),
    ClientErrorPage   = (1 << 3)
} ClientFlags;

typedef struct {
    ClientFlags flags;
    Window xid;
    guint64 pageid;
    GtkWidget *widget;
    WebKitWebView *view;
    WebKitWebInspector *inspector;
    WebKitFindController *finder;
    WebKitHitTestResult *mousepos;
    GTlsCertificate *cert;
    GTlsCertificate *failedcert;
    GTlsCertificateFlags tlserr;
#ifdef FEATURE_TITLE    
    const char *title;
    const char *overtitle;
    int progress;
#endif  /* FEATURE_TITLE */    
    const char *targeturi;
    const char *needle;
    ClientCallback cb;
} Client;


extern Atom atom_rc;
extern const char *user_agent;


gboolean clients_init (void);
void clients_free (void);
Client * clients_find (uint64 pageid);

Client * client_new (void);
Client * client_new_view (WebKitWebView *view);
void client_free (Client *c);

void client_view_signals_connect (WebKitWebView *v, Client *c);
void client_view_signals_disconnect (Client *c);
void client_view_context_signals_connect (WebKitWebContext *context, Client *c);
void client_view_context_signals_disconnect (Client *c);

Client * clients_prepend (void);
Client * clients_prepend_view (WebKitWebView *view);
void clients_shift (void);
GList * client_remove (Client *c);

void clients_reload (gboolean bypass_cache);
void client_reload (Client *c, gboolean bypass_cache);

void client_uri_load (Client *c, const char *uri);
void client_atom_set (Window win, Atom atom, const char *val);
void client_atom_update_rc (Client *c, const char *uri);
CommandType client_atom_rc_get (Client *c, RunCommandParameters *rcparams);

void client_script_run_file (Client *c, const char *file);
void client_script_run (Client *c, const char *script);
void client_script_eval (Client *c, const char *jsstr, ...);

void client_handle_plumb (Client *c, const char *uri);
void client_window_new (Client *c,  const char *uri, gboolean noembed);
void client_spawn (Client *c, const char **argv);
void client_spawn_rc (Client *c, const char **argv, int argc, CommandType rc);
void client_view_create (Client *c);
void client_download (Client *c, WebKitURIResponse *r);
void client_view_show (Client *c);
void client_navigate (Client *c, gboolean forward);

void client_execute_token (Client *c, ChannelToken token, const gchar *msg);
void client_verbose_stats (Client *c);

#ifdef FEATURE_FULLSCREEN
void client_toggle_fullscreen (Client *c);
#endif

#ifdef FEATURE_TITLE
void client_update_title (Client *c);
#endif


#endif /* _CLIENT_H_ */
