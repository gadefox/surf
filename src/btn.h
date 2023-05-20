/* See LICENSE file for copyright and license details. */

#ifndef _BTN_H_
#define _BTN_H_

#include <webkit2/webkit2.h>
#include "key.h"


typedef enum {
    TargetDoc       = WEBKIT_HIT_TEST_RESULT_CONTEXT_DOCUMENT,
    TargetLink      = WEBKIT_HIT_TEST_RESULT_CONTEXT_LINK,
    TargetImage     = WEBKIT_HIT_TEST_RESULT_CONTEXT_IMAGE,
    TargetMedia     = WEBKIT_HIT_TEST_RESULT_CONTEXT_MEDIA,
    TargetEdit      = WEBKIT_HIT_TEST_RESULT_CONTEXT_EDITABLE,
    TargetScrollbar = WEBKIT_HIT_TEST_RESULT_CONTEXT_SCROLLBAR,
    TargetSelection = WEBKIT_HIT_TEST_RESULT_CONTEXT_SELECTION,
    TargetAny       = TargetDoc | TargetLink | TargetImage | TargetMedia | TargetEdit | TargetScrollbar | TargetSelection
} Target;

typedef enum {
    MouseLeft = 1,
    MouseMiddle,
    MouseRight,
    MouseScrollUp,
    MouseScrollDown
} Mouse;

typedef struct {
    const char *name;
    Target target;
    Event event;
    Mouse mouse;
    void (*func)(Client *c, const Arg a, WebKitHitTestResult *h);
    gboolean stopevent;
    Arg arg;
} Button;


void btn_navigate (Client *c, const Arg a, WebKitHitTestResult *h);
void btn_newwindow (Client *c, const Arg a, WebKitHitTestResult *h);
void btn_play (Client *c, const Arg a, WebKitHitTestResult *h);

gboolean btn_callback_released (GtkWidget *w, GdkEvent *e, Client *c);

void btn_mouse_verbose (Mouse mouse);
void btn_target_verbose (Target target);
void btn_verbose (Button *btn, uint mlen);
void btns_verbose (void);


#endif /* _BTN_H_ */
