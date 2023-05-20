/* See LICENSE file for copyright and license details. */

#include <sys/socket.h>
#include <gdk/gdk.h>
#include <gtk/gtkx.h>

#include "webkit.h"
#include "verbose.h"
#include "serialize.h"
#include "parameter.h"
#include "key.h"
#include "surf.h"
#include "callback.h"
#include "string.h"


static GdkDevice *gdkkb;

const char *wk_prefix_http = "http://";
const char *wk_prefix_https = "https://";
const char *wk_prefix_about = "about:";
const char *wk_prefix_file = "file://";
const char *wk_prefix_data = "data:";
const char *wk_prefix_blob = "blob:";

Window wk_embed;
gboolean wk_show_xid;

int wk_sock_in = -1;


void
webkit_init (void)
{
    GdkDisplay *gdpy;
    int spair [2];
    GdkSeat *gseat;

    gtk_init (NULL, NULL);

    gdpy = gdk_display_get_default ();
    gseat = gdk_display_get_default_seat (gdpy);
    gdkkb = gdk_seat_get_keyboard (gseat);

    if ( socketpair (AF_UNIX, SOCK_DGRAM, 0, spair) == -1 )
        error ("unable to create sockets");
    else {
        channel_init (spair [0], webkit_read_message);
        wk_sock_in = spair [1];
    }
}

void
webkit_socket_free (void)
{
    channel_socket_free ();

    close (wk_sock_in);
    wk_sock_in = -1;
}

gboolean
webkit_read_message (GIOChannel *channel, GIOCondition condition, gpointer unused)
{
    ChannelMessage chmsg;
    Client *client;

    /* read io channel message */
    if ( channel_read (channel, &chmsg) ) {
        /* find the client */
        client = clients_find (chmsg.pageid);
        if ( client == NULL )
            error ("cannot find client: %llu", chmsg.pageid);
        else
            /* handle the message */
            client_execute_token (client, chmsg.token, chmsg.msg);

        /* free allocated buffer */
        g_free (chmsg.msg);
    }

    /* TRUE means don't remove the event source */
    return TRUE;
}

void
webkit_handle_find_in_page (WebKitFindController *finder, FindInPageParameters *fp)
{
    const char *find;

    find = webkit_find_controller_get_search_text (finder);
    if ( find != NULL && strcmp (find, fp->keyword) == 0 )  /* reset search */
        webkit_find_controller_search (finder, "", fp->options, G_MAXUINT);

    if ( *fp->keyword != '\0' )
        webkit_find_controller_search (finder, fp->keyword, fp->options, G_MAXUINT);
    else
        webkit_find_controller_search_finish (finder);
}

void
webkit_handle_user_password (Client *c, UserPasswordParameters* up)
{
    char *msg;
    uint len;

    /* serialize binary structure using 6bit encoding */
    msg = serialize_user_password_msg (c->pageid, up, &len);
    if ( msg != NULL ) {
        /* send the message to the web extension */
        channel_send (msg, len);

        /* free the buffer */
        free (msg);
    }
}

void
webkit_handle_go (Client *c, GoParameters *gp)
{
    client_uri_load (c, gp->uri);
}

void
webkit_execute_run_command (Client *c, CommandType rctype, RunCommandParameters *rcparams)
{
#ifdef DUMP  
    info ("rc: executed: %d", rctype);
#endif  /* DUMP */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
    switch ( rctype )
      {
        case CommandGo:
            webkit_handle_go (c, (GoParameters *) rcparams);
            break;

        case CommandFindInPage:
            webkit_handle_find_in_page (c->finder, (FindInPageParameters *) rcparams);
            break;

        case CommandUserPassword:
            webkit_handle_user_password (c, (UserPasswordParameters *) rcparams);
            break;
      }
#pragma GCC diagnostic pop
}

GdkFilterReturn
webkit_handle_xevents (XEvent *xevent, GdkEvent *event, Client *c)
{
    CommandType rctype;
    RunCommandParameters rcparams;
    
    if ( xevent->type != PropertyNotify || xevent->xproperty.state != PropertyNewValue )
        return GDK_FILTER_CONTINUE;
       
    if ( xevent->xproperty.atom == atom_rc )
      {
        /* parse json */
        rctype = client_atom_rc_get (c, &rcparams);
        if ( rctype != CommandUndefined ) {
            /* execure run command */
            webkit_execute_run_command (c, rctype, &rcparams);

            /* free allocated structure */
            run_command_params_free (rctype, &rcparams);
        }
        return GDK_FILTER_REMOVE;
      }
         
    return GDK_FILTER_CONTINUE;
}

#ifdef FEATURE_TITLE

gboolean
webkit_event_notify_enter (GtkWidget *w, GdkEvent *e, Client *c)
{
    c->overtitle = c->targeturi;
    client_update_title (c);
    return FALSE;
}

gboolean
webkit_event_notify_leave (GtkWidget *w, GdkEvent *e, Client *c)
{
    c->overtitle = NULL;
    client_update_title (c);
    return FALSE;
}

#endif  /* FEATURE_TITLE */

#ifdef FEATURE_FULLSCREEN

gboolean
webkit_event_win_state (GtkWidget *w, GdkEvent *e, Client *c)
{
    if ( e->window_state.changed_mask != GDK_WINDOW_STATE_FULLSCREEN )
        return FALSE;

    SETBIT (c->flags, ClientFullscreen, e->window_state.new_window_state & GDK_WINDOW_STATE_FULLSCREEN);
    return FALSE;
}

#endif  /* FEATURE_FULLSCREEN */

gboolean
webkit_event_key_press (GtkWidget *w, GdkEvent *e, Client *c)
{
    return key_handle_press (c, e);
}

static void
webkit_window_set_role (Client *c, GtkWindow *win)
{
    String wmstr;

    s_init (&wmstr, 16);
    s_add_str (&wmstr, prog_name, -1);
    s_add_char (&wmstr, '[');
    s_add_uint64 (&wmstr, c->pageid);
    s_add_char (&wmstr, ']');
    if ( s_close (&wmstr) )
        gtk_window_set_role (win, wmstr.head);
    s_free (&wmstr);
}

void
webkit_window_create (Client *c)
{
    GtkWindow *win;
    GtkWidget *widget;

    if ( wk_embed ) {
        widget = gtk_plug_new (wk_embed);
    } else {
        widget = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        win = GTK_WINDOW (widget);

        gtk_window_set_wmclass (win, prog_name, prog_name);
        webkit_window_set_role (c, win);

#if defined (WIN_WIDTH) && defined (WIN_HEIGHT)
        gtk_window_set_default_size (win, WIN_WIDTH, WIN_HEIGHT);
#endif
    }

    callback_widget_signals_connect (widget, c);
    c->widget = widget;
}

void
webkit_decide_navigation (WebKitPolicyDecision *d, Client *c)
{
    WebKitNavigationPolicyDecision *nd;
    WebKitNavigationAction *action;
    WebKitNavigationType type;

    nd = WEBKIT_NAVIGATION_POLICY_DECISION (d);
    action = webkit_navigation_policy_decision_get_navigation_action (nd);
    type = webkit_navigation_action_get_navigation_type (action);

    switch ( type )
      {
        case WEBKIT_NAVIGATION_TYPE_LINK_CLICKED:
            /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_FORM_SUBMITTED:
            /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_BACK_FORWARD:
            /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_RELOAD:
            /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_FORM_RESUBMITTED:
            /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_OTHER:
            /* fallthrough */
        default:
            /* Do not navigate to links with a "_blank" target (popup) */
            if ( webkit_navigation_policy_decision_get_frame_name (nd) )
                webkit_policy_decision_ignore (d);
            else
                /* Filter out navigation to different domain ? get action→urirequest, copy and load in new window+view on Ctrl+Click ? */
                webkit_policy_decision_use (d);
            break;
      }
}

void
webkit_decide_new_window (WebKitPolicyDecision *d, Client *c)
{
    const char *uri;
    WebKitNavigationPolicyDecision *nd;
    WebKitNavigationAction *action;
    WebKitNavigationType type;
    WebKitURIRequest *req;

    nd = WEBKIT_NAVIGATION_POLICY_DECISION (d);
    action = webkit_navigation_policy_decision_get_navigation_action (nd);
    type = webkit_navigation_action_get_navigation_type (action);

    switch ( type )
      {
        case WEBKIT_NAVIGATION_TYPE_LINK_CLICKED:
            /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_FORM_SUBMITTED:
            /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_BACK_FORWARD:
            /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_RELOAD:
            /* fallthrough */
        case WEBKIT_NAVIGATION_TYPE_FORM_RESUBMITTED:
            /* Filter domains here.  If the value of “mouse-button” is not 0, then the navigation was triggered by a mouse event.
             * Test for link clicked but no button ? */
            req = webkit_navigation_action_get_request (action);
            uri = webkit_uri_request_get_uri (req);
            client_window_new (c, uri, FALSE);
            break;

        case WEBKIT_NAVIGATION_TYPE_OTHER:
            /* fallthrough */
        default:
            break;
      }

    webkit_policy_decision_ignore (d);
}

bool
webkit_uri_is_ascii (const gchar *uri)
{
    const gchar *s;
    gchar c;

    for ( s = uri, c = *s; c != '\0'; c = *++s ) {
        if ( !g_ascii_isprint (c) )
            return FALSE;
    }
   
    return s != uri;
}

void
webkit_decide_resource (WebKitPolicyDecision *d, Client *c)
{
    WebKitResponsePolicyDecision *r;
    WebKitURIResponse *res;
    const gchar *uri;
   
    /* init */
    r = WEBKIT_RESPONSE_POLICY_DECISION (d);
    res = webkit_response_policy_decision_get_response (r);
    uri = webkit_uri_response_get_uri (res);

    if ( g_str_has_suffix(uri, "/favicon.ico") ) {
        webkit_policy_decision_ignore(d);
        return;
    }

    if ( !g_str_has_prefix (uri, wk_prefix_http)
         && !g_str_has_prefix (uri, wk_prefix_https)
         && !g_str_has_prefix (uri, wk_prefix_about)
         && !g_str_has_prefix (uri, wk_prefix_file)
         && !g_str_has_prefix (uri, wk_prefix_data)
         && !g_str_has_prefix (uri, wk_prefix_blob)
         && webkit_uri_is_ascii (uri) ) {
        client_handle_plumb (c, uri);
        webkit_policy_decision_ignore (d);
        return;
    }

    if ( webkit_response_policy_decision_is_mime_type_supported (r) )
        webkit_policy_decision_use (d);
    else {
        webkit_policy_decision_ignore (d);
        client_download (c, res);
    }
}

void
webkit_window_destroy (GtkWidget* w, Client *c)
{
    /* disconnect signals */
    callback_widget_signals_disconnect (w, c);

    /* destroy the client and remove the container from the list */
    if ( client_remove (c) == NULL )
        gtk_main_quit ();
}

WebKitCookieAcceptPolicy
webkit_cookie_policy_get (int cookie_policy)
{
    const char *cookie_policies;
    char val;

    cookie_policies = parameter_get_string (CookiePolicies);
    val = cookie_policies [cookie_policy];

    switch (val) {
        case 'a':
            return WEBKIT_COOKIE_POLICY_ACCEPT_NEVER;

        case '@':
            return WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY;

        default:
            /* fallthrough */
        case 'A':
            return WEBKIT_COOKIE_POLICY_ACCEPT_ALWAYS;
    }
}

char
webkit_cookie_policy_to_char (const WebKitCookieAcceptPolicy p)
{
    switch (p) {
        case WEBKIT_COOKIE_POLICY_ACCEPT_NEVER:
            return 'a';
 
        case WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY:
            return '@';
 
        default:
             /* fallthrough */
        case WEBKIT_COOKIE_POLICY_ACCEPT_ALWAYS:
            return 'A';
    }
}

const char *
webkit_cookie_policy_to_string (char p)
{
    switch (p) {
        case 'a':
            return "never";

        case '@':
            return "no 3rd party";

        default:
            /* fallthrough */
        case 'A':
            return "always";
    }
}
