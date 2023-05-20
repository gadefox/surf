/* See LICENSE file for copyright and license details. */

#ifndef _CALLBACK_H_
#define _CALLBACK_H_

#include <glib.h>


typedef struct {
    /* view */
    gulong released;
#ifdef FEATURE_TITLE
    gulong progress_changed;
    gulong title_changed;
#endif  /* FEATURE_TITLE */    
    gulong insecure_content;
    gulong view_close;
    gulong view_create;
    gulong decide_policy;
    gulong load_failed_tls;
    gulong load_changed;
    gulong mouse_target_changed;
    gulong permission_requested;
    gulong view_show;
    gulong web_process_terminated;
    /* gtk widget */
    gulong win_destroy;
    gulong event_key_press;
#ifdef FEATURE_FULLSCREEN
    gulong event_win_state;
#endif  /* FEATURE_FULLSCREEN */
#ifdef FEATURE_TITLE
    gulong event_enter_notify;
    gulong event_leave_notify;
#endif  /* FEATURE_TITLE */
    /* web context */
    gulong download_started;
    gulong init_web_extensions;
    /* webkit download */
    gulong response_received; 
} ClientCallback;


extern gboolean cb_show_xid;


#include "client.h"


GtkWidget * callback_view_create (WebKitWebView *v, WebKitNavigationAction *a, Client *c);
void callback_view_close (WebKitWebView *v, Client *c);
void callback_view_show (WebKitWebView *v, Client *c);

void callback_web_process_terminated (WebKitWebView *v, WebKitWebProcessTerminationReason r, Client *c);
gboolean callback_decide_policy (WebKitWebView *v, WebKitPolicyDecision *d, WebKitPolicyDecisionType dt, Client *c);
gboolean callback_permission_requested (WebKitWebView *v, WebKitPermissionRequest *r, Client *c);
void callback_mouse_target_changed (WebKitWebView *v, WebKitHitTestResult *h, guint modifiers, Client *c);
void callback_load_changed (WebKitWebView *v, WebKitLoadEvent e, Client *c);
gboolean callback_load_failed_tls (WebKitWebView *v, gchar *uri, GTlsCertificate *cert, GTlsCertificateFlags err, Client *c);
void callback_download_started (WebKitWebContext *wc, WebKitDownload *d, Client *c);
void callback_response_received (WebKitDownload *d, GParamSpec *ps, Client *c);
void callback_init_web_extensions (WebKitWebContext *wc, Client *c);

void callback_view_signals_connect (WebKitWebView *v, Client *c);
void callback_view_signals_disconnect (Client *c);

void callback_view_context_signals_connect (WebKitWebContext *context, Client *c);
void callback_view_context_signals_disconnect (Client *c);

void callback_widget_signals_connect (GtkWidget *w, Client *c);
void callback_widget_signals_disconnect (GtkWidget *w, Client *c);

void callback_insecure_content (WebKitWebView *v, WebKitInsecureContentEvent e, Client *c);

#ifdef FEATURE_TITLE
void callback_title_changed (WebKitWebView *view, GParamSpec *ps, Client *c);
void callback_progress_changed (WebKitWebView *v, GParamSpec *ps, Client *c);
#endif  /* FEATURE_TITLE */


#endif /* _CALLBACK_H_ */
