/* See LICENSE file for copyright and license details. */

#include <gcr/gcr.h>

#include "key.h"
#include "client.h"
#include "surf.h"
#include "verbose.h"
#include "file.h"
#include "btn.h"
#include "arg.h"
#include "webkit.h"
#include "serialize.h"


const char caret_browsing_name [] = "caret browsing";
const char geolocation_name [] = "geolocation";
const char disk_cache_name [] = "disk cache";
const char load_images_name [] = "load images";
const char notifications_name [] = "notifications";
const char style_name [] = "style";
const char certificate_name [] = "certificate";
const char strict_tls_name [] = "strict TLS";
const char hw_acceleration_name [] = "hardware acceleration";
const char https_name [] = "https";
const char tlserr_name [] = "TLS error";
const char client_insecure_name [] = "client insecure";

const char key_forward_name [] = "forward";
const char key_back_name [] = "back";

char key_toggle_stats [11];

/* hotkeys: If you use anything else but EM_Any, EM_ModKey and EM_ModShift, don't forget to edit the 'ToEventMask' macro. */
static Key keys [] = {
   /* name                    EventMask          keyval              function             Arg */
    { "go",                   EventModKey,       GDK_KEY_g,          key_spawn_rc,      { .c = CommandGo } },
    { "find in page",         EventModKey,       GDK_KEY_f,          key_spawn_rc,      { .c = CommandFindInPage } },
    { "search engine",        EventModKey,       GDK_KEY_s,          key_spawn_rc,      { .c = CommandSearchEngine } },
    { "user/password",        EventModKey,       GDK_KEY_space,      key_spawn_rc,      { .c = CommandUserPassword } },
        
    { "stop",                 EventAny,          GDK_KEY_Escape,     key_stop           /* unused */ },
    { "quit",                 EventModKey,       GDK_KEY_q,          key_quit           /* unused */ },
    
    { "reload no cache",      EventModKeyShift,  GDK_KEY_r,          key_reload,        { .b = TRUE } },
    { "reload",               EventModKey,       GDK_KEY_r,          key_reload,        { .b = FALSE } },
    
    { key_forward_name,       EventModKey,       GDK_KEY_backslash,  key_navigate,      { .b = TRUE } },
    { key_back_name,          EventModKey,       GDK_KEY_BackSpace,  key_navigate,      { .b = FALSE } },
    
    { "zoom in",              EventModKey,       GDK_KEY_Next,       key_zoom,          { .i = -1 } },
    { "zoom out",             EventModKey,       GDK_KEY_Prior,      key_zoom,          { .i = +1 } },
    
    { "copy",                 EventModKey,       GDK_KEY_c,          key_clipboard,     { .b = TRUE } },
    { "paste",                EventModKey,       GDK_KEY_v,          key_clipboard,     { .b = FALSE } },
    
    { "find next",            EventModKey,       GDK_KEY_n,          key_find_in_page,  { .b = TRUE } },
    { "find prev",            EventModKeyShift,  GDK_KEY_n,          key_find_in_page,  { .b = FALSE } },

    { "print",                EventModKeyShift,  GDK_KEY_p,          key_print          /* unused */ },
    { "show cert",            EventModKey,       GDK_KEY_t,          key_showcert       /* unused */ },
    
    { "cookie policy",        EventModKeyShift,  GDK_KEY_a,          key_toggle_cookie_policy,  /* unused */ },
    { "inspector",            EventModKeyShift,  GDK_KEY_o,          key_toggle_inspector  /* unused */ },
    
    { caret_browsing_name,    EventModKeyShift,  GDK_KEY_b,          key_toggle,        { .p = CaretBrowsing } },
    { geolocation_name,       EventModKeyShift,  GDK_KEY_g,          key_toggle,        { .p = Geolocation } },
    { load_images_name,       EventModKeyShift,  GDK_KEY_i,          key_toggle,        { .p = LoadImages } },
    { notifications_name,     EventModKeyShift,  GDK_KEY_l,          key_toggle,        { .p = Notifications } },
    { strict_tls_name,        EventModKeyShift,  GDK_KEY_t,          key_toggle,        { .p = StrictTLS } },
    { "scrollbars",           EventModKeyShift,  GDK_KEY_s,          key_toggle,        { .p = ScrollBars } }
};


void
key_init (void)
{
    key_toggle_stats [10] = '\0';
    key_update_toggle_stats ();
}

void
key_event_verbose (Event event)
{
    /* control */
    if ( event & GDK_CONTROL_MASK )
        verbose_s ("ctrl+");

    /* shift */
    if ( event & GDK_SHIFT_MASK )
        verbose_s ("shift+");
}

void
key_verbose_key (Key *key, uint mlen)
{
    const char *keyname;

    /* name */
    verbose_color_prefix (key->name, VerboseMagenda);

    /* spaces */
    mlen -= strlen (key->name);
    verbose_spaces (mlen);

    /* event mask */
    key_event_verbose (key->event);

    /* key */
    keyname = gdk_keyval_name (key->keyval);
    verbose_s (keyname);

    /* newline */
    verbose_newline ();
}

static uint
keys_verbose_get_max_len (void)
{
    uint max = 0;
    uint len, i;
    Key *key;

    for ( i = 0, key = keys; i < countof (keys); i++, key++ ) {
        len = strlen (key->name);
        if ( max < len )
            max = len;
    }
    return max;
}

void
keys_verbose (void)
{
    uint i, mlen;
    Key *key;

    info ("keys");
    mlen = keys_verbose_get_max_len ();

    for ( i = 0, key = keys; i < countof (keys); i++, key++ )
        key_verbose_key (key, mlen);
}

void
key_spawn (Client *c, const Arg a)
{
    client_spawn (c, (const char **) a.v);
}

void
key_spawn_rc (Client *c, const Arg a)
{
    channel_send ("ahoj", 5);
//    client_spawn_rc (c, NULL, 0, a.c);
}

void
key_uri_paste (GtkClipboard *clipboard, const char *text, gpointer d)
{
    if ( text != NULL )
        client_uri_load ((Client *) d, text);
}

void
key_reload (Client *c, const Arg a)
{
    client_reload (c, a.b);
}

void
key_print (Client *c, const Arg a)
{
    WebKitPrintOperation *print_op;
    GtkWindow *win;

    print_op = webkit_print_operation_new (c->view);
    win = GTK_WINDOW (c->widget);
    webkit_print_operation_run_dialog (print_op, win);
}

void
key_showcert (Client *c, const Arg a)
{
    GTlsCertificate *cert;
    GcrCertificate *gcrt;
    GByteArray *crt;
    GtkWidget *win;
    GcrCertificateWidget *wcert;

    cert = c->failedcert ? c->failedcert : c->cert;
    if ( cert == NULL )
        return;

    g_object_get(cert, "certificate", &crt, NULL);
    gcrt = gcr_simple_certificate_new(crt->data, crt->len);
    g_byte_array_unref(crt);

    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    wcert = gcr_certificate_widget_new(gcrt);
    g_object_unref(gcrt);

    gtk_container_add(GTK_CONTAINER(win), GTK_WIDGET(wcert));
    gtk_widget_show_all(win);
}

void
key_clipboard (Client *c, const Arg a)
{
    GtkClipboard *clipboard;
    const char *uri;

    clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
    if ( a.b )  /* load clipboard uri */
        gtk_clipboard_request_text (clipboard, key_uri_paste, c);
    else {  /* copy uri */
        if ( c->targeturi != NULL )
            uri = c->targeturi;
        else
            uri = webkit_web_view_get_uri (c->view);
        gtk_clipboard_set_text (clipboard, uri, -1);
    }
}

void
key_zoom (Client *c, const Arg a)
{
    Parameter *p = parameter_get (ZoomLevel);
    if (a.i > 0)
        webkit_web_view_set_zoom_level (c->view, p->val.f + 0.1);
    else if (a.i < 0)
        webkit_web_view_set_zoom_level (c->view, p->val.f - 0.1);
    else  /* a.i == 0 */
        webkit_web_view_set_zoom_level (c->view, 1.0);

    p->val.f = webkit_web_view_get_zoom_level (c->view);
}

#ifdef FEATURE_SCROLL

static void
key_scroll (Client *c, gboolean vert, int val)
{
    uint len;
    char *msg;

    msg = serialize_scroll_msg (c->pageid, vert, val, &len);
    if ( msg != NULL ) {
        channel_send (msg, len);
        free (msg);
    }
}

void
key_scrollv (Client *c, const Arg a)
{
    key_scroll (c, TRUE, a.i);
}

void
key_scrollh (Client *c, const Arg a)
{
    key_scroll (c, FALSE, a.i);
}

#endif  /* FEATURE_SCROLL */

void
key_navigate (Client *c, const Arg a)
{
    client_navigate (c, a.b);
}

void
key_stop (Client *c, const Arg a)
{
    webkit_web_view_stop_loading (c->view);
}

void
key_quit (Client *c, const Arg a)
{
    gtk_main_quit ();
}

void
key_verbose (Client *c, const Arg a)
{
    /* keys */
    if ( a.vi & VerboseKeys )
        keys_verbose ();

    /* buttons */
    if ( a.vi & VerboseButtons )
        btns_verbose ();

    /* toggle stats */
    if ( a.vi & VerboseToggleStats )
        key_verbose_toggle_stats ();
}

void
key_toggle (Client *c, const Arg a)
{
    Parameter *p = parameter_get (a.p);
    p->val.b ^= TRUE;
    parameter_set (c, TRUE, a.p, p->val);

    key_update_toggle_stats ();
}

#ifdef FEATURE_FULLSCREEN

void
key_toggle_fullscreen (Client *c, const Arg a)
{
    client_toggle_fullscreen (c);
}

#endif  /* FEATURE_FULLSCREEN */

void
key_toggle_cookie_policy (Client *c, const Arg a)
{
    parameter_toggle_cookie_policy (c);
    key_update_toggle_stats ();
}

void
key_toggle_inspector (Client *c, const Arg a)
{
    if (webkit_web_inspector_is_attached(c->inspector))
        webkit_web_inspector_close(c->inspector);
    else if ( parameter_is (Inspector) )
        webkit_web_inspector_show(c->inspector);
}

void
key_find_in_page (Client *c, const Arg a)
{
    if ( a.b )
        webkit_find_controller_search_next (c->finder);
    else
        webkit_find_controller_search_previous (c->finder);
}

gboolean
key_handle_press (Client *c, GdkEvent *e)
{
    uint i;
    Key *key;

    if ( parameter_is (KioskMode) )
        return FALSE;

    for ( i = 0, key = keys; i < countof (keys); i++, key++ ) {
        if ( key->event != ToEvent (e->key.state) )
            continue;

        if ( gdk_keyval_to_lower (e->key.keyval) != key->keyval )
            continue;

        /* Please note the 'func' vector can't be null by the definition */
        key->func (c, key->arg);
        return TRUE;
    }

    return FALSE;
}

void
key_update_toggle_stats (void)
{
    WebKitCookieAcceptPolicy cookie_policy;
    char *s;

    s = key_toggle_stats;
    
    /* cookie policy */
    cookie_policy = webkit_cookie_policy_get (param_cookie_policy);
    *s++ = webkit_cookie_policy_to_char (cookie_policy);  /* 0 */

    /* other */
    *s++ = parameter_is (CaretBrowsing) ? 'B' : 'b';  /* 1 */
    *s++ = parameter_is (Geolocation)   ? 'G' : 'g';  /* 2 */
    *s++ = parameter_is (DiskCache)     ? 'D' : 'd';  /* 3 */
    *s++ = parameter_is (LoadImages)    ? 'I' : 'i';  /* 4 */
    *s++ = parameter_is (Notifications) ? 'L' : 'l';  /* 5 */
    *s++ = parameter_is (Style)         ? 'S' : 's';  /* 6 */
    *s++ = parameter_is (Certificate)   ? 'C' : 'c';  /* 7 */
    *s++ = parameter_is (StrictTLS)     ? 'T' : 't';  /* 8 */
    *s   = parameter_get_uint (HWAcceleration) != WEBKIT_HARDWARE_ACCELERATION_POLICY_NEVER
                                        ? 'A' : 'a';  /* 9 */
}

void
key_verbose_toggle_stats (void)
{
    const char *cookie_policy;
    char *s;

    s = key_toggle_stats;

    /* overview */
    info ("toggle stats");

    /* cookie policy */
    cookie_policy = webkit_cookie_policy_to_string (*s++);
    verbose_string ("cookie policy", cookie_policy);

    /* other */
    verbose_boolean (caret_browsing_name,  *s++ == 'B');
    verbose_boolean (geolocation_name,     *s++ == 'G');
    verbose_boolean (disk_cache_name,      *s++ == 'D');
    verbose_boolean (load_images_name,     *s++ == 'I');
    verbose_boolean (notifications_name,   *s++ == 'L');
    verbose_boolean (style_name,           *s++ == 'S');
    verbose_boolean (certificate_name,     *s++ == 'C');
    verbose_boolean (strict_tls_name,      *s++ == 'T');
    verbose_boolean (hw_acceleration_name, *s   == 'A');
}
