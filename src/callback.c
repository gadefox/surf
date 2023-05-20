/* See LICENSE file for copyright and license details. */

#include "callback.h"
#include "client.h"
#include "parameter.h"
#include "file.h"
#include "verbose.h"
#include "webkit.h"
#include "btn.h"


gboolean cb_show_xid = FALSE;


GtkWidget *
callback_view_create (WebKitWebView *v, WebKitNavigationAction *a, Client *c)
{
    Client *new_client;
    WebKitNavigationType type;

    type = webkit_navigation_action_get_navigation_type (a);

    switch ( type ) {
        case WEBKIT_NAVIGATION_TYPE_OTHER:
            /* Popup windows of type “other” are almost always triggered by user gesture, so inverse the logic here
             * Instead of this, compare destination uri to mouse-over uri for validating window */
            if ( webkit_navigation_action_is_user_gesture (a) )
                return NULL;
            /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_LINK_CLICKED:
            /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_FORM_SUBMITTED:
            /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_BACK_FORWARD:
            /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_RELOAD:
            /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_FORM_RESUBMITTED:
            break;

        default:
            return NULL;
    }

    new_client = clients_prepend_view (c->view);
    if ( new_client == NULL )
        return NULL;

    return GTK_WIDGET (new_client->view);
}

void
callback_view_show (WebKitWebView *v, Client *c)
{
    client_view_show (c);
}

gboolean
callback_load_failed_tls (WebKitWebView *v, gchar *uri, GTlsCertificate *cert, GTlsCertificateFlags err, Client *c)
{
    GString *errmsg;
    gchar *html, *pem;

    warn ("loaded failed TLS");

    errmsg = g_string_new (NULL);

    c->failedcert = g_object_ref(cert);
    c->tlserr = err;
    c->flags |= ClientErrorPage;

    if (err & G_TLS_CERTIFICATE_UNKNOWN_CA)
        g_string_append (errmsg, "The signing certificate authority is not known.<br>");
    if (err & G_TLS_CERTIFICATE_BAD_IDENTITY)
        g_string_append (errmsg, "The certificate does not match the expected identity of the site that it was retrieved from.<br>");
    if (err & G_TLS_CERTIFICATE_NOT_ACTIVATED)
        g_string_append (errmsg, "The certificate's activation time is still in the future.<br>");
    if (err & G_TLS_CERTIFICATE_EXPIRED)
        g_string_append (errmsg, "The certificate has expired.<br>");
    if (err & G_TLS_CERTIFICATE_REVOKED)
        g_string_append (errmsg, "The certificate has been revoked according to the GTlsConnection's certificate revocation list.<br>");
    if (err & G_TLS_CERTIFICATE_INSECURE)
        g_string_append (errmsg, "The certificate's algorithm is considered insecure.<br>");
    if (err & G_TLS_CERTIFICATE_GENERIC_ERROR)
        g_string_append (errmsg, "Some error occurred validating the certificate.<br>");

    g_object_get (cert, "certificate-pem", &pem, NULL);
    html = g_strdup_printf ("<p>Could not validate TLS for “%s”<br>%s</p>"
                            "<p>You can inspect the following certificate "
                            "with Ctrl-t (default keybinding).</p>"
                            "<p><pre>%s</pre></p>", uri, errmsg->str, pem);
    g_free (pem);
    g_string_free (errmsg, TRUE);

    webkit_web_view_load_alternate_html (c->view, html, uri, NULL);
    g_free (html);

    return TRUE;
}

void
callback_load_changed (WebKitWebView *v, WebKitLoadEvent e, Client *c)
{
    const char *uri;
   
    uri = webkit_web_view_get_uri (c->view);

    switch (e)
      {
        case WEBKIT_LOAD_STARTED:
#ifdef FEATURE_TITLE
            c->title = uri;
#endif  /* FEATURE_TITLE */
            c->flags &= ~(ClientHttps | ClientInsecure);
            uriparameters_transient_set (c, uri);
            if ( c->flags & ClientErrorPage )
                c->flags &= ~ClientErrorPage;
            else
                g_clear_object (&c->failedcert);
            break;

        case WEBKIT_LOAD_REDIRECTED:
#ifdef FEATURE_TITLE
            c->title = uri;
#endif  /* FEATURE_TITLE */
            uriparameters_transient_set (c, uri);
            break;

        case WEBKIT_LOAD_COMMITTED:
#ifdef FEATURE_TITLE
            c->title = uri;
#endif  /* FEATURE_TITLE */
            uriparameters_committed_set (c, uri);
            if ( webkit_web_view_get_tls_info (c->view, &c->cert, &c->tlserr) )
                c->flags |= ClientHttps;
            else
                c->flags &= ~ClientHttps;
            break;

        case WEBKIT_LOAD_FINISHED:
            uriparameters_finished_set (c, uri);
//            client_atom_update_rc (c, uri);
            client_script_run_file (c, file_script);
//            client_spawn_rc (c, NULL, 0, CommandPageLoaded);
            break;
      }

#ifdef FEATURE_TITLE
    client_update_title (c);
#endif
}

void
callback_mouse_target_changed (WebKitWebView *v, WebKitHitTestResult *h, guint modifiers, Client *c)
{
    WebKitHitTestResultContext hit_result = webkit_hit_test_result_get_context (h);

    /* Keep the hit test to know where is the pointer on the next click */
    c->mousepos = h;

    if ( hit_result & WEBKIT_HIT_TEST_RESULT_CONTEXT_LINK )
        c->targeturi = webkit_hit_test_result_get_link_uri (h);
    else if ( hit_result & WEBKIT_HIT_TEST_RESULT_CONTEXT_IMAGE )
        c->targeturi = webkit_hit_test_result_get_image_uri (h);
    else if ( hit_result & WEBKIT_HIT_TEST_RESULT_CONTEXT_MEDIA )
        c->targeturi = webkit_hit_test_result_get_media_uri (h);
    else
        c->targeturi = NULL;

#ifdef FEATURE_TITLE    
    c->overtitle = c->targeturi;
    client_update_title (c);
#endif  /* FEATURE_TITLE */
}

static ParameterType
callback_permission_get_param (WebKitPermissionRequest *r)
{
    WebKitUserMediaPermissionRequest *umr;

    if ( WEBKIT_IS_GEOLOCATION_PERMISSION_REQUEST (r) )
        return Geolocation;

    if ( WEBKIT_IS_USER_MEDIA_PERMISSION_REQUEST (r) ) {
        umr = WEBKIT_USER_MEDIA_PERMISSION_REQUEST (r);
        if ( webkit_user_media_permission_is_for_audio_device (umr) )
            return AccessMicrophone;
        
        if ( webkit_user_media_permission_is_for_video_device (umr) )
            return AccessWebcam;

        return ParameterLast;
    }
 
    if ( WEBKIT_IS_NOTIFICATION_PERMISSION_REQUEST (r))
        return Notifications;

    return ParameterLast;
}

gboolean
callback_permission_requested (WebKitWebView *v, WebKitPermissionRequest *r, Client *c)
{
    ParameterType param;

    param = callback_permission_get_param (r);
    if ( param == ParameterLast )
        return FALSE;

    if ( parameter_is (param) )
        webkit_permission_request_allow (r);
    else
        webkit_permission_request_deny (r);

    return TRUE;
}

gboolean
callback_decide_policy (WebKitWebView *v, WebKitPolicyDecision *d, WebKitPolicyDecisionType dt, Client *c)
{
    switch (dt) {
    case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION:
        webkit_decide_navigation (d, c);
        break;

    case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION:
        webkit_decide_new_window (d, c);
        break;

    case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
        webkit_decide_resource (d, c);
        break;

    default:
        webkit_policy_decision_ignore (d);
        break;
    }
    return TRUE;
}

void
callback_web_process_terminated (WebKitWebView *v, WebKitWebProcessTerminationReason r, Client *c)
{
    warn ("web process terminated: %s", r == WEBKIT_WEB_PROCESS_CRASHED ? "crashed" : "no memory");
    callback_view_close (v, c);
}

void
callback_view_close (WebKitWebView *v, Client *c)
{
    gtk_widget_destroy (c->widget);
}

void
callback_download_started (WebKitWebContext *wc, WebKitDownload *d, Client *c)
{
    GObject *o;
 
    info ("started download");

    /* connect signal */
    o = G_OBJECT (d);
    c->cb.response_received = g_signal_connect (o, "notify::response", G_CALLBACK (callback_response_received), c);
}

void
callback_init_web_extensions (WebKitWebContext *wc, Client *c)
{
    GVariant *gv;

    if ( wk_sock_in == -1 )
        return;

    gv = g_variant_new (G_GINT32_FORMAT, wk_sock_in);

    webkit_web_context_set_web_extensions_initialization_user_data (wc, gv);
    webkit_web_context_set_web_extensions_directory (wc, WEBEXT_DIR);
}

void
callback_response_received (WebKitDownload *d, GParamSpec *ps, Client *c)
{
    WebKitURIResponse *r;
    GObject *o;

    info ("received response");

    /* disconnect signal */
    o = G_OBJECT (d);
    g_signal_handler_disconnect (o, c->cb.response_received);

    /* save downloaded file */
    r = webkit_download_get_response (d);
    client_download (c, r);
    webkit_download_cancel (d);
}

void
callback_view_signals_connect (WebKitWebView *v, Client *c)
{
    GObject *o;

    o = G_OBJECT (v);

    /* buttons */
    c->cb.released = g_signal_connect (o, "button-release-event", G_CALLBACK (btn_callback_released), c);

    /* view */
    c->cb.view_close = g_signal_connect (o, "close", G_CALLBACK (callback_view_close), c);
    c->cb.view_create = g_signal_connect (o, "create", G_CALLBACK (callback_view_create), c);
    c->cb.decide_policy = g_signal_connect (o, "decide-policy", G_CALLBACK (callback_decide_policy), c);
    c->cb.load_failed_tls = g_signal_connect (o, "load-failed-with-tls-errors", G_CALLBACK (callback_load_failed_tls), c);
    c->cb.load_changed = g_signal_connect (o, "load-changed", G_CALLBACK (callback_load_changed), c);
    c->cb.mouse_target_changed = g_signal_connect (o, "mouse-target-changed", G_CALLBACK (callback_mouse_target_changed), c);
    c->cb.permission_requested = g_signal_connect (o, "permission-request", G_CALLBACK (callback_permission_requested), c);
    c->cb.view_show = g_signal_connect (o, "ready-to-show", G_CALLBACK (callback_view_show), c);
    c->cb.web_process_terminated = g_signal_connect (o, "web-process-terminated", G_CALLBACK (callback_web_process_terminated), c);
    c->cb.insecure_content = g_signal_connect (o, "insecure-content-detected", G_CALLBACK (callback_insecure_content), c);

#ifdef FEATURE_TITLE
    c->cb.progress_changed = g_signal_connect (o, "notify::estimated-load-progress", G_CALLBACK (callback_progress_changed), c);
    c->cb.title_changed = g_signal_connect (o, "notify::title", G_CALLBACK (callback_title_changed), c);
#endif  /* FEATURE_TITLE */
}

void
callback_view_signals_disconnect (Client *c)
{
    GObject *o;

    o = G_OBJECT (c->view);
    g_signal_handler_disconnect (o, c->cb.released);
    g_signal_handler_disconnect (o, c->cb.view_close);
    g_signal_handler_disconnect (o, c->cb.view_create);
    g_signal_handler_disconnect (o, c->cb.decide_policy);
    g_signal_handler_disconnect (o, c->cb.load_failed_tls);
    g_signal_handler_disconnect (o, c->cb.load_changed);
    g_signal_handler_disconnect (o, c->cb.mouse_target_changed);
    g_signal_handler_disconnect (o, c->cb.permission_requested);
    g_signal_handler_disconnect (o, c->cb.view_show);
    g_signal_handler_disconnect (o, c->cb.web_process_terminated);
    g_signal_handler_disconnect (o, c->cb.insecure_content);

#ifdef FEATURE_TITLE
    g_signal_handler_disconnect (o, c->cb.progress_changed);
    g_signal_handler_disconnect (o, c->cb.title_changed);
#endif  /* FEATURE_TITLE */

    callback_view_context_signals_disconnect (c);
}

void
callback_view_context_signals_disconnect (Client *c)
{
    GObject *o;
    WebKitWebContext *webcontext;

    webcontext = webkit_web_view_get_context (c->view);
    o = G_OBJECT (webcontext);

    if ( c->cb.download_started != 0 )
        g_signal_handler_disconnect (o, c->cb.download_started);

    if ( c->cb.init_web_extensions != 0 )
        g_signal_handler_disconnect (o, c->cb.init_web_extensions);
}

void
callback_view_context_signals_connect (WebKitWebContext *context, Client *c)
{
    GObject *o;
    
    o = G_OBJECT (context);

    c->cb.download_started = g_signal_connect (o, "download-started", G_CALLBACK (callback_download_started), c);
    c->cb.init_web_extensions = g_signal_connect (o, "initialize-web-extensions", G_CALLBACK (callback_init_web_extensions), c);
}

void
callback_widget_signals_connect (GtkWidget *w, Client *c)
{
    GObject *o;

    o = G_OBJECT (w);

    c->cb.win_destroy = g_signal_connect (o, "destroy", G_CALLBACK(webkit_window_destroy), c);
    c->cb.event_key_press = g_signal_connect (o, "key-press-event", G_CALLBACK(webkit_event_key_press), c);

#ifdef FEATURE_FULLSCREEN
    c->cb.event_win_state = g_signal_connect (o, "window-state-event", G_CALLBACK(webkit_event_win_state), c);
#endif  /* FEATURE_FULLSCREEN */

#ifdef FEATURE_TITLE
    c->cb.event_enter_notify = g_signal_connect (o, "enter-notify-event", G_CALLBACK(webkit_event_notify_enter), c);
    c->cb.event_leave_notify = g_signal_connect (o, "leave-notify-event", G_CALLBACK(webkit_event_notify_leave), c);
#endif  /* FEATURE_TITLE */
}

void
callback_widget_signals_disconnect (GtkWidget* w, Client *c)
{
    GObject *o;

    o = G_OBJECT (w);
    g_signal_handler_disconnect (o, c->cb.win_destroy);
    g_signal_handler_disconnect (o, c->cb.event_key_press);

#ifdef FEATURE_FULLSCREEN
    g_signal_handler_disconnect (o, c->cb.event_win_state);
#endif  /* FEATURE_FULLSCREEN */

#ifdef FEATURE_TITLE
    g_signal_handler_disconnect (o, c->cb.event_enter_notify);
    g_signal_handler_disconnect (o, c->cb.event_leave_notify);
#endif  /* FEATURE_TITLE */
}

void
callback_insecure_content (WebKitWebView *v, WebKitInsecureContentEvent e, Client *c)
{
    warn ("insecure content");
    c->flags |= ClientInsecure;
}

#ifdef FEATURE_TITLE

void
callback_progress_changed (WebKitWebView *v, GParamSpec *ps, Client *c)
{
    c->progress = webkit_web_view_get_estimated_load_progress (c->view) * 100;
    client_update_title (c);
}

void
callback_title_changed (WebKitWebView *view, GParamSpec *ps, Client *c)
{
    c->title = webkit_web_view_get_title (c->view);
    client_update_title (c);
}

#endif  /* FEATURE_TITLE */
