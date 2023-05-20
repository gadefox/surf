/* See LICENSE file for copyright and license details. */

#include <sys/stat.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <glib.h>

#include "callback.h"
#include "webkit.h"
#include "btn.h"
#include "parameter.h"
#include "key.h"
#include "verbose.h"
#include "parameter.h"
#include "surf.h"
#include "jsonrc.h"
#include "file.h"
#include "string.h"


#define PROGRESS  100


static GList *clients = NULL;
static Display *dpy = NULL;
static Atom atom_uri;

Atom atom_rc;
const char *user_agent;


gboolean
clients_init (void)
{
    /* display */
    dpy = XOpenDisplay (NULL);
    if ( dpy == NULL ) {
        error ("can't open default display");
        return FALSE;
    }

    /* atoms */
    atom_uri = XInternAtom (dpy, "_SURF_URI", False);
    atom_rc = XInternAtom (dpy, "_SURF_RC", False);
    return TRUE;
}

Client *
client_new (void)
{
    Client *c;

    c = calloc (1, sizeof (Client));
    if ( c == NULL )
        return NULL;

#ifdef FEATURE_TITLE    
    c->progress = PROGRESS;
#endif

    /* create view */
    client_view_create (c);
    return c;
}

Client *
client_new_view (WebKitWebView *v)
{
    Client *c;
    GtkWidget *widget;
    WebKitWebView *view;

    c = calloc (1, sizeof (Client));
    if ( c == NULL )
        return NULL;

#ifdef FEATURE_TITLE
    c->progress = PROGRESS;
#endif
 
    /* create view from an existing one and init signals */
    widget = webkit_web_view_new_with_related_view (v);
    view = WEBKIT_WEB_VIEW (widget);
    callback_view_signals_connect (view, c);

    c->view = view;
    return c;
}

Client *
clients_prepend (void)
{
    Client *c;

    c = client_new ();
    if ( c == NULL )
        return NULL;

    clients = g_list_prepend (clients, c);
    return c;
}

Client *
clients_prepend_view (WebKitWebView *v)
{
    Client *c;

    c = client_new_view (v);
    if ( c == NULL )
        return NULL;

    clients = g_list_prepend (clients, c);
    return c;
}

void
client_uri_load (Client *c, const char *uri)
{
    struct stat st;
    gchar *path;
    String new_uri;

    /* is the uri empty? */
    if ( *uri == '\0' )
        return;

    if ( g_str_has_prefix (uri, wk_prefix_http)  ||
         g_str_has_prefix (uri, wk_prefix_https) ||
         g_str_has_prefix (uri, wk_prefix_file) ||
         g_str_has_prefix (uri, wk_prefix_about)) {
        /* just copy the uri pointer */
        new_uri.head = (char *) uri;
        new_uri.flags &= ~StringOwnsBuffer;
    } else {
        path = dir_untilde (uri);
        if ( path == NULL )
            return;

        /* build new uri */
        s_init (&new_uri, 48);

        if ( stat (path, &st) == 0 ) {
            s_add_str (&new_uri, wk_prefix_file, -1);
            s_add_str (&new_uri, path, -1);
        } else {
            s_add_str (&new_uri, wk_prefix_http, -1);
            s_add_str (&new_uri, uri, -1);
        }
        g_free (path);

        /* check the error flag and terminate the string w/ '\0' */
        if ( !s_close (&new_uri) ) {
            s_free (&new_uri);
            return;
        }
    }

    uri = webkit_web_view_get_uri (c->view);
    if ( uri != NULL && strcmp (uri, new_uri.head) == 0 )
        client_reload (c, TRUE);
    else
        webkit_web_view_load_uri (c->view, new_uri.head);

    s_free (&new_uri);
}

void
client_atom_set (Window win, Atom atom, const char *val)
{
    uint count = 1;

    if ( val != NULL )
        count += strlen (val);
    else
        val = "";

    XChangeProperty (dpy, win, atom, XA_STRING, 8, PropModeReplace, (byte *) val, count);
    XSync (dpy, False);
}

static char *
client_atom_get (Client *c, Atom atom)
{
    Atom adummy;
    int idummy;
    ulong ldummy;
    char *val = NULL;

    /* get property */
    XSync (dpy, False);
    XGetWindowProperty (dpy, c->xid, atom, 0, G_MAXLONG, False, XA_STRING, &adummy, &idummy, &ldummy, &ldummy, (byte **) &val);
   
    return val;
}

CommandType
client_atom_rc_get (Client *c, RunCommandParameters *rcparams)
{
    char *json;
    CommandType rctype;

    /* get json */
    json = client_atom_get (c, atom_rc);
    if ( json == NULL )
        return CommandUndefined;

    /* parse json */
    rctype = json_parse_run_command (json, rcparams);

    /* free allocated buffer */
    XFree (json);
    return rctype;
}

void
client_verbose_stats (Client *c)
{
    info ("page stats");

    if ( c->flags & ClientHttps ) {
        verbose_boolean (https_name, TRUE);
        verbose_boolean (tlserr_name, c->tlserr != 0);
        verbose_boolean (client_insecure_name, c->flags & ClientInsecure);
    } else
        verbose_boolean (https_name, FALSE);
}

void
client_script_run_file (Client *c, const char *file)
{
    gchar *script;
    gsize len;

    if ( file == NULL || !g_file_get_contents (file, &script, &len, NULL) )
        return;
    
    if ( len != 0 )
        client_script_run (c, script);
    
    g_free (script);
}

void
client_script_eval (Client *c, const char *jsstr, ...)
{
    va_list ap;
    gchar *script;

    va_start(ap, jsstr);
    script = g_strdup_vprintf (jsstr, ap);
    va_end(ap);

    client_script_run (c, script);
    
    g_free(script);
}

void
client_script_run (Client *c, const char *script)
{
    webkit_web_view_run_javascript (c->view, script, NULL, NULL, NULL);
}

void
client_handle_plumb (Client *c, const char *uri)
{
    client_spawn_rc (c, &uri, 1, CommandPlumb);
}

void
client_free (Client *c)
{
    callback_view_signals_disconnect (c);
    webkit_web_view_stop_loading (c->view);
    free (c);
}

GList *
client_remove (Client *c)
{
    clients = g_list_remove (clients, c);
    client_free (c);
    return clients;
}

void
clients_free (void)
{
    /* destroy client list */
    g_list_free_full (clients, (GDestroyNotify) client_free);

    /* display */
    if ( dpy != NULL )
        XCloseDisplay (dpy);
}
   
Client *
clients_find (uint64 pageid)
{
    GList *iter;
    Client *client;

    for ( iter = clients; iter != NULL; iter = iter->next ) {
        client = iter->data;
        if ( client->pageid == pageid )
            return client;
    }
    return NULL;
}

void
client_window_new (Client *c, const char *uri, gboolean noembed)
{
    char szuint [UINT_MAX_SIZE];
    const char *cmd [29];  /* ATTN: check the vector size at the end of this function! */
    const char **v;

    v = cmd;
    *v++ = surf_argv0;                                     /* 0 */
    *v++ = "-a";                                           /* 1 */

    *v++ = parameter_get_string (CookiePolicies);          /* 2 */
    *v++ = parameter_is (ScrollBars) ? "-B" : "-b";        /* 3 */

    if ( file_cookie != NULL ) {
        *v++ = "-c";                                       /* 4 */
        *v++ = file_cookie;                                /* 5 */
    }

    if ( surf_style_file != NULL ) {
        *v++ = "-C";                                       /* 6 */
        *v++ = surf_style_file;                            /* 7 */
    }

    *v++ = parameter_is (DiskCache) ? "-D" : "-d";         /* 8 */

    if ( wk_embed && !noembed ) {
        *v++ = "-e";                                       /* 9 */
        s_uint (szuint, wk_embed);
        *v++ = szuint;                                     /* 10 */
    }

    *v++ = parameter_is (RunInFullscreen) ? "-F" : "-f" ;  /* 11 */
    *v++ = parameter_is (Geolocation) ?     "-G" : "-g" ;  /* 12 */
    *v++ = parameter_is (LoadImages) ?      "-I" : "-i" ;  /* 13 */
    *v++ = parameter_is (KioskMode) ?       "-K" : "-k" ;  /* 14 */
    *v++ = parameter_is (Notifications) ?   "-L" : "-l" ;  /* 15 */
    *v++ = parameter_is (Style) ?           "-M" : "-m" ;  /* 16 */
    *v++ = parameter_is (Inspector) ?       "-N" : "-n" ;  /* 17 */

    if ( file_script != NULL ) {
        *v++ = "-r";                                       /* 18 */
        *v++ = file_script;                                /* 19 */
    }

    *v++ = parameter_is (StrictTLS) ?        "-T" : "-t";  /* 20 */

    if ( parameter_get_uint (HWAcceleration) == WEBKIT_HARDWARE_ACCELERATION_POLICY_NEVER )
        *v++ = "-A";                                       /* 21 */

    if ( surf_full_user_agent != NULL ) {
        *v++ = "-u";                                       /* 22 */
        *v++ = surf_full_user_agent;                       /* 23 */
    }

    if ( wk_show_xid )
        *v++ = "-w";                                       /* 24 */

    *v++ = parameter_is (Certificate) ? "-X" : "-x" ;      /* 25 */

    /* do not keep zoom level */
    *v++ = "--";                                           /* 26 */

    if ( uri != NULL )
        *v++ = uri;                                        /* 27 */

    *v = NULL;                                             /* 28 */
                                                       /* # = 29 */
    client_spawn (c, cmd);
}

void
client_spawn (Client *c, const char **v)
{
    const char *s;

    if ( fork() != 0 )
        return;

    if ( dpy != NULL )
        close (ConnectionNumber (dpy));

    webkit_socket_free ();
    setsid ();

#ifdef DUMP    
    info ("spawn");
    verbose_v (v);
#endif  /* DUMP */

    s = *v;
    execvp (s, (char *const *)v);

    perror ("failed");
    error ("execvp %s", s);

    /* quit the application */
    gtk_main_quit ();
}

void
client_atom_update_rc (Client *c, const char *uri)
{
    gchar *json_s;

    /* create json string */
    if ( uri == NULL )
        uri = webkit_web_view_get_uri (c->view);
 
    json_s = json_build_webpage_info (uri);

#ifdef DUMP    
    info (json_s);
#endif  /* DUMP */

    /* set atom value */
    client_atom_set (c->xid, atom_uri, json_s);

    /* free json string */
    g_free (json_s);
}

void
client_spawn_rc (Client *c, const char **argv, int argc, CommandType rc)
{
    const char *args [16];  /* ATTN: check max arguments */
    const char **dst;
    const char *s;
    char szrc [UINT_MAX_SIZE];
    char winid [UINT_HEX_SIZE];

    /* run command */
    dst = args;
    if ( rc != CommandUndefined ) {
        /* surf run command script */
        *dst++ = SURF_RC;        /* rc: 0 */

        /* win id */
        s_hex (winid, c->xid);
        *dst++ = winid;          /* rc: 1 */

        /* run command type */
        s_uint (szrc, rc);
        *dst++ = szrc;           /* rc: 2 */
    } else {
        /* or shell */
        *dst++ = SHELL;          /* sh: 0 */
        *dst++ = "-c";           /* sh: 1 */
    }

    /* copy source vector */
    if ( argc < 0 ) {
        /* unknown vector size, vector is terminated with NULL string */
        for ( s = *argv; s != NULL; s = *++argv, dst++ )
            *dst = s;
    } else {
        /* known vector size */
        while ( argc-- != 0 )
            *dst++ = *argv++;
    }

    /* terminate vector */
    *dst = NULL;
    
    /* spawn */
    client_spawn (c, args);
}

void
client_view_create (Client *c)
{
    WebKitWebView *view;
    WebKitSettings *settings;
    WebKitWebContext *context;
    WebKitCookieManager *cookiemanager;
    WebKitUserContentManager *contentmanager;
    WebKitCookieAcceptPolicy cookiepolicy;
    WebKitWebsiteDataManager *datamanager;
    
    /* Webview: For more interesting settings, have a look at http://webkitgtk.org/reference/webkit2gtk/stable/WebKitSettings.html */
    settings = webkit_settings_new_with_settings(
            "allow-file-access-from-file-urls", parameter_is (FileURLsCrossAccess),
            "allow-universal-access-from-file-urls", parameter_is (FileURLsCrossAccess),
            "auto-load-images", parameter_is (LoadImages),
            "default-charset", parameter_get_string (DefaultCharset),
            "default-font-size", parameter_get_int (FontSize),
            "enable-caret-browsing", parameter_is (CaretBrowsing),
            "enable-developer-extras", parameter_is (Inspector),
            "enable-dns-prefetching", parameter_is (DNSPrefetch),
            "enable-html5-database", parameter_is (DiskCache),
            "enable-html5-local-storage", parameter_is (DiskCache),
            "enable-javascript", parameter_is (JavaScript),
            "enable-site-specific-quirks", parameter_is (SiteQuirks),
            "enable-smooth-scrolling", parameter_is (SmoothScrolling),
            "enable-webgl", parameter_is (WebGL),
            "hardware-acceleration-policy", parameter_get_uint (HWAcceleration),
            "media-playback-requires-user-gesture", parameter_is (MediaManualPlay),
            NULL);

    if ( surf_is_user_agent ) {
        if ( surf_full_user_agent != NULL )
            webkit_settings_set_user_agent (settings, surf_full_user_agent);
        else
            webkit_settings_set_user_agent_with_application_details (settings, prog_name, VERSION);
    }

    user_agent = webkit_settings_get_user_agent (settings);
    contentmanager = webkit_user_content_manager_new ();

    if ( parameter_is (Ephemeral) )
        context = webkit_web_context_new_ephemeral ();
    else {
        datamanager = webkit_website_data_manager_new(
                         "base-cache-directory", dir_cache,
                         "base-data-directory", dir_cache,
                         NULL);
        context = webkit_web_context_new_with_website_data_manager (datamanager);
    }

    cookiemanager = webkit_web_context_get_cookie_manager (context);
    
    /* rendering process model, can be a shared unique one or one for each view */
    webkit_web_context_set_process_model (context, WEBKIT_PROCESS_MODEL_MULTIPLE_SECONDARY_PROCESSES);

    /* TLS */
    webkit_website_data_manager_set_tls_errors_policy (datamanager,
        parameter_is (StrictTLS) ? WEBKIT_TLS_ERRORS_POLICY_FAIL : WEBKIT_TLS_ERRORS_POLICY_IGNORE);

    /* disk cache */
    webkit_web_context_set_cache_model (context,
        parameter_is (DiskCache) ? WEBKIT_CACHE_MODEL_WEB_BROWSER : WEBKIT_CACHE_MODEL_DOCUMENT_VIEWER);

    /* Currently only works with text file to be compatible with curl */
    if ( file_cookie != NULL )
        webkit_cookie_manager_set_persistent_storage (cookiemanager, file_cookie, WEBKIT_COOKIE_PERSISTENT_STORAGE_TEXT);

    /* cookie policy */
    cookiepolicy = webkit_cookie_policy_get (param_cookie_policy);
    webkit_cookie_manager_set_accept_policy (cookiemanager, cookiepolicy);

    /* languages */
    webkit_web_context_set_preferred_languages (context, (const gchar *const *) parameter_get_vector (PreferredLanguages) );
    webkit_web_context_set_spell_checking_languages (context, (const gchar *const *) parameter_get_vector (SpellLanguages) );
    webkit_web_context_set_spell_checking_enabled (context, parameter_is (SpellChecking) );

    /* set client view */
    view = g_object_new (WEBKIT_TYPE_WEB_VIEW,
                "settings", settings,
                "user-content-manager", contentmanager,
                "web-context", context,
                NULL);

    callback_view_context_signals_connect (context, c);
    callback_view_signals_connect (view, c);

    c->view = view;
}

#ifdef FEATURE_FULLSCREEN

void
client_toggle_fullscreen (Client *c)
{
    GtkWindow *w;
   
    w = GTK_WINDOW (c->widget);
    if ( c->flags & ClientFullscreen )
        gtk_window_unfullscreen (w);
    else
        gtk_window_fullscreen (w);
}

#endif  /* FEATURE_FULLSCREEN */

void
client_download (Client *c, WebKitURIResponse *r)
{
    const char *response;
   
    response  = webkit_uri_response_get_uri (r);
    client_spawn_rc (c, &response, 1, CommandDownload);
}

void
clients_reload (gboolean bypass_cache)
{
    GList *iter;

    for ( iter = clients; iter != NULL; iter = iter->next )
        client_reload (iter->data, bypass_cache);
}

void
client_reload (Client *c, gboolean bypass_cache)
{
    if ( bypass_cache )
        webkit_web_view_reload_bypass_cache (c->view);
    else
        webkit_web_view_reload (c->view);
}

void client_view_show (Client *c)
{
    GdkRGBA bgcolor = { 0 };
    GdkWindow *win;
    float tmpf;
    WebKitWebView *view;
    GtkWidget *widget_view, *widget_win;
    GdkDisplay * gdpy;
    char winid [UINT_HEX_SIZE];

    view = c->view;
    widget_view = GTK_WIDGET (view);
    c->finder = webkit_web_view_get_find_controller (view);
    c->inspector = webkit_web_view_get_inspector (view);
    c->pageid = webkit_web_view_get_page_id (view);
    
    webkit_window_create (c);
    widget_win = c->widget;

    gtk_container_add (GTK_CONTAINER (widget_win), widget_view);
    gtk_widget_show_all (widget_win);
    gtk_widget_grab_focus (widget_view);

    win = gtk_widget_get_window (widget_win);
    c->xid = gdk_x11_window_get_xid (win);

    if ( cb_show_xid ) {
        gdpy = gtk_widget_get_display (widget_win);
        gdk_display_sync (gdpy);

        s_hex (winid, c->xid);
        puts (winid);

        fflush (stdout);
    }

    if ( parameter_is (HideBackground) )
        webkit_web_view_set_background_color (view, &bgcolor);

    if ( !parameter_is (KioskMode) ) {
        gdk_window_set_events (win, GDK_ALL_EVENTS_MASK);
        gdk_window_add_filter (win, (GdkFilterFunc) webkit_handle_xevents, c);
    }

#ifdef FEATURE_FULLSCREEN    
    if ( parameter_is (RunInFullscreen) )
        client_toggle_fullscreen (c);
#endif  /* FEATURE_FULLSCREEN */

    tmpf = parameter_get_float (ZoomLevel);
    if ( tmpf != 1.0 )
        webkit_web_view_set_zoom_level (view, tmpf);
}

void
client_navigate (Client *c, gboolean forward)
{
    if ( forward )
        webkit_web_view_go_forward (c->view);
    else
        webkit_web_view_go_back (c->view);
}

void
client_execute_token (Client *c, ChannelToken token, const gchar *msg)
{
    switch ( token )
      {
        default:
            warn ("unknown token: %u", token);
            break;
      }
}

#ifdef FEATURE_TITLE

void
client_update_title (Client *c)
{
    char *title, *s;
    char page_stats [4];
    GtkWindow *win;
    const char *name;
   
    name = c->overtitle != NULL ? c->overtitle : c->title != NULL ? c->title : "";
    win = GTK_WINDOW (c->widget);

    if ( parameter_get (ShowIndicators) ) {
        /* build stats */
        s = page_stats;
        if ( c->flags & ClientHttps ) {
            *s++ =                             's';        /* 0 */
            *s++ = c->tlserr != 0            ? 'e' : ' ';  /* 1 */
            *s++ = c->flags & ClientInsecure ? 'i' : ' ';  /* 2 */
        } else {
            *s++ = 'h';  /* 0 */
            *s++ = ' ';  /* 1 */
            *s++ = ' ';  /* 2 */
        }
        *s = '\0';

        if ( c->progress != 100 )
            title = g_strdup_printf ("[%i%%] %s:%s | %s", c->progress, key_toggle_stats, page_stats, name);
        else
            title = g_strdup_printf ("%s:%s | %s", key_toggle_stats, page_stats, name);

        gtk_window_set_title (win, title);
        g_free (title);
    } else
        gtk_window_set_title (win, name);
}

#endif  /* FEATURE_TITLE */
