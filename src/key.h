/* See LICENSE file for copyright and license details. */

#ifndef _KEY_H_
#define _KEY_H_

#include "arg.h"
#include "callback.h"


#define EventMask   (GDK_CONTROL_MASK | GDK_SHIFT_MASK)
#define ToEvent(e)  ((e) & EventMask)

typedef enum {
    EventAny         = 0,
    EventModKey      = GDK_CONTROL_MASK,
    EventModKeyShift = GDK_CONTROL_MASK | GDK_SHIFT_MASK,
    EventModKeyCtrl  = GDK_CONTROL_MASK
} Event;

typedef struct {
    const char *name;
    Event event;
    guint keyval;
    void (*func)(Client *c, const Arg a);
    const Arg arg;
} Key;


extern const char caret_browsing_name [];
extern const char geolocation_name [];
extern const char disk_cache_name [];
extern const char load_images_name [];
extern const char notifications_name [];
extern const char style_name [];
extern const char certificate_name [];
extern const char strict_tls_name [];
extern const char hw_acceleration_name [];
extern const char https_name [];
extern const char tlserr_name [];
extern const char client_insecure_name [];

extern const char key_forward_name [];
extern const char key_back_name[ ];

extern char key_toggle_stats [11];


void key_init (void);

void key_reload (Client *c, const Arg a);
void key_print (Client *c, const Arg a);
void key_showcert (Client *c, const Arg a);
void key_clipboard (Client *c, const Arg a);
void key_zoom (Client *c, const Arg a);

#ifdef FEATURE_SCROLL
void key_scrollv (Client *c, const Arg a);
void key_scrollh (Client *c, const Arg a);
#endif

void key_navigate (Client *c, const Arg a);
void key_stop (Client *c, const Arg a);
void key_quit (Client *c, const Arg a);
void key_verbose (Client *c, const Arg a);
void key_toggle (Client *c, const Arg a);

#ifdef FEATURE_FULLSCREN
void key_toggle_fullscreen (Client *c, const Arg a);
#endif

void key_toggle_cookie_policy (Client *c, const Arg a);
void key_toggle_inspector (Client *c, const Arg a);
void key_find_in_page (Client *c, const Arg a);
void key_search_engine (Client *c, const Arg a);
void key_spawn_rc (Client *c, const Arg a);

gboolean key_handle_press (Client *c, GdkEvent *e);
void key_uri_paste (GtkClipboard *clipboard, const char *text, gpointer d);
void keys_verbose (void);
void key_verbose_key (Key *key, uint mlen);
void key_event_verbose (Event event);

void key_update_toggle_stats (void);
void key_verbose_toggle_stats (void);


#endif /* _KEY_H_ */
