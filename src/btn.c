/* See LICENSE file for copyright and license details. */

#include "btn.h"
#include "verbose.h"


/* button definitions */
/* target can be OnDoc, OnLink, OnImg, OnMedia, OnEdit, OnBar, OnSel, OnAny */
static Button buttons [] = {
   /* name               Target        Event         Mouse         function        stop event Arg */
    { "new window",      TargetLink,   EventAny,     MouseMiddle,     btn_newwindow,  TRUE,    { .b = FALSE } },
    { "window embeded",  TargetLink,   EventModKey,  MouseLeft,       btn_newwindow,  TRUE,    { .b = TRUE } },

    { key_forward_name,  TargetAny,    EventAny,     MouseScrollUp,   btn_navigate,   TRUE,    { .b = FALSE } },
    { key_back_name,     TargetAny,    EventAny,     MouseScrollDown, btn_navigate,   TRUE,    { .b = TRUE } },

    { "play",            TargetMedia,  EventModKey,  MouseLeft,       btn_play,       TRUE     /* unused */ }
};

static const char button_name [] = "button";


void
btn_target_verbose (Target target)
{
    gboolean used = FALSE;

    verbose_s ("on ");
    if ( target & TargetDoc )       {                                          used = TRUE;   verbose_s ("document"); }
    if ( target & TargetLink )      { if ( used ) { verbose_comma (); } else { used = TRUE; } verbose_s ("link"); }
    if ( target & TargetImage )     { if ( used ) { verbose_comma (); } else { used = TRUE; } verbose_s ("image"); }
    if ( target & TargetMedia )     { if ( used ) { verbose_comma (); } else { used = TRUE; } verbose_s ("media"); }
    if ( target & TargetEdit )      { if ( used ) { verbose_comma (); } else { used = TRUE; } verbose_s ("edit"); }
    if ( target & TargetScrollbar ) { if ( used ) { verbose_comma (); } else { used = TRUE; } verbose_s ("scrollbar"); }
    if ( target & TargetSelection ) { if ( used ) { verbose_comma (); } else { used = TRUE; } verbose_s ("selection"); }
}

static void
btn_mouse_verbose_nr (uint nr)
{
    verbose_s (button_name);
    verbose_i (nr);
}

static void
btn_mouse_verbose_postfix (const char *s)
{
    verbose_s (s);
    verbose_c (' ');
    verbose_s (button_name);
}

void
btn_mouse_verbose (Mouse mouse)
{
    switch ( mouse )
    {
        case MouseLeft:
            btn_mouse_verbose_postfix ("left");
            break;

        case MouseMiddle:
            btn_mouse_verbose_postfix ("middle");
            break;

        case MouseRight:
            btn_mouse_verbose_postfix ("right");
            break;

        case MouseScrollUp:
            btn_mouse_verbose_postfix ("scroll up");
            break;

        case MouseScrollDown:
            btn_mouse_verbose_postfix ("scroll down");
            break;
 
        default:
            btn_mouse_verbose_nr (mouse);
            break;
   }
}

void
btn_verbose (Button *btn, uint mlen)
{
    /* name */
    verbose_color_prefix (btn->name, VerboseMagenda);

    /* spaces */
    mlen -= strlen (btn->name);
    verbose_spaces (mlen);

    /* event mask and button */
    key_event_verbose (btn->event);
    btn_mouse_verbose (btn->mouse);
    verbose_c (' ');

    /* target */
    btn_target_verbose (btn->target);

    /* newline */
    verbose_newline ();
}

static uint
btns_verbose_get_max_len (void)
{
    uint max = 0;
    uint len, i;
    Button *btn;

    for ( i = 0, btn = buttons; i < countof (buttons); i++, btn++ ) {
        len = strlen (btn->name);
        if ( max < len )
            max = len;
    }
    return max;
}

void
btns_verbose (void)
{
    uint i, mlen;
    Button *btn;
   
    info ("mouse");
    mlen = btns_verbose_get_max_len ();

    for ( i = 0, btn = buttons; i < countof (buttons); i++, btn++ )
        btn_verbose (btn, mlen);
}

void
btn_navigate (Client *c, const Arg a, WebKitHitTestResult *h)
{
    client_navigate (c, a.b);
}

void
btn_newwindow (Client *c, const Arg a, WebKitHitTestResult *h)
{
    const char *uri;

    uri = webkit_hit_test_result_get_link_uri(h);
    client_window_new (c, uri, a.b);
}

void
btn_play (Client *c, const Arg a, WebKitHitTestResult *h)
{
    const char *uri;
   
    uri = webkit_hit_test_result_get_media_uri (h);
    client_spawn_rc (c, &uri, 1, CommandPlayVideo);
}

gboolean
btn_callback_released (GtkWidget *w, GdkEvent *e, Client *c)
{
    WebKitHitTestResultContext element;
    uint i;
    Button *btn;
    
    element = webkit_hit_test_result_get_context (c->mousepos);
    
    for ( i = 0, btn = buttons; i < countof (buttons); i++, btn++ ) {
        if ( btn->mouse != e->button.button )
            continue;
        
        if ( (btn->target & element) == 0 )
            continue;

        if ( btn->event != ToEvent (e->button.state) )
            continue;

        /* vector 'func' can't be null by definition */
        btn->func (c, btn->arg, c->mousepos);
        return btn->stopevent;
    }

    return FALSE;
}
