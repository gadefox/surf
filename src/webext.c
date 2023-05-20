/* See LICENSE file for copyright and license details. */

#include <webkit2/webkit-web-extension.h>

#include "verbose.h"
#include "channel.h"


static WebKitWebExtension *web_ext;

static const char input_name[] = "INPUT";


static void
element_set_next_input_value (WebKitDOMDocument* document, WebKitDOMElement* user, const gchar *value)
{
    WebKitDOMNodeList *inputs = webkit_dom_document_get_elements_by_tag_name (document, input_name);
    gulong count = webkit_dom_node_list_get_length (inputs);
    int found = FALSE;

    for ( gulong i = 0; i < count; i++ ) {
        WebKitDOMNode *node = webkit_dom_node_list_item (inputs, i);
        WebKitDOMElement *element = WEBKIT_DOM_ELEMENT (node);
        if ( element == user )
            found = TRUE;
        else if ( found ) {
            webkit_dom_element_html_input_element_set_editing_value (element, value);
            return;
        }
    }
}

static void
webext_handle_user_password (WebKitWebPage *page, const gchar *msg)
{
    char *value, *tagname;
    WebKitDOMDocument* document;
    WebKitDOMElement* element;

    /* deserialize user (string) */
    msg = encoding_deserialize_string (msg, &value, NULL);
    if ( msg == NULL )
        return;

#ifdef DUMP
    verbose_string ("user", value);
#endif

    /* init */
    document = webkit_web_page_get_dom_document (page);
    element = webkit_dom_document_get_active_element (document);
    tagname = webkit_dom_element_get_tag_name (element);
    if ( strcmp (tagname, input_name) != 0 )
        return;

    webkit_dom_element_html_input_element_set_editing_value (element, value);
    free (value);

    /* deserialize password (string) */
    msg = encoding_deserialize_string (msg, &value, NULL);
    if ( msg == NULL )
        return;

#ifdef DUMP
    verbose_string ("password", value);
#endif

    element_set_next_input_value (document, element, value);
    free (value);
}

#ifdef FEATURE_SCROLL

static void
webext_handle_scroll (WebKitWebPage *page, const gchar *msg)
{
    gboolean vert;
    int value;
    static char jscript [48];
    WebKitFrame *frame;
    JSCContext *jsctx;
    
    /* init */
    frame = webkit_web_page_get_main_frame (page);
    jsctx = webkit_frame_get_js_context (frame);

    /* deserialize $vert (boolean) and $value (int12) */
    vert = encoding_deserialize_boolean (*msg++);
    encoding_deserialize_int_char2 (msg, &value);
    value %= 100;  /* buffer protection */

    /* scroll page */
    if ( vert )
        snprintf (jscript, countof (jscript), "window.scrollBy(0,window.innerHeight/100*%d);", value);
    else
        snprintf (jscript, countof (jscript), "window.scrollBy(window.innerWidth/100*%d,0);", value);
    
    jsc_context_evaluate (jsctx, jscript, -1);
}

#endif  /* FEATURE_SCROLL */

static void
webext_execute_token (WebKitWebPage *page, ChannelToken token, const gchar *msg)
{
    /* check the run command */
    switch ( token )
      {
#ifdef FEATURE_SCROLL          
        case ChannelTokenScroll:
            webext_handle_scroll (page, msg);
            break;
#endif  /* FEATURE_SCROLL */

        case ChannelTokenUserPassword:
            webext_handle_user_password (page, msg);
            break;

        /* add new run commands here */

        default:
            warn ("unknown token: %u", token);
            break;
      }
}

static gboolean
webext_message_read (GIOChannel *channel, GIOCondition condition, gpointer unused)
{
    verbose_i ((int)condition);
    verbose_s("imin\n");

    ChannelMessage chmsg;
    WebKitWebPage *page;
        page = webkit_web_extension_get_page (web_ext, 10);
        verbose_s("_4");
        if (page != NULL)
          verbose_s ("_5");
        else
          verbose_s ("_6");

    /* read io channel message */
    if ( channel_read (channel, &chmsg) ) {
      verbose_s("_7");
        /* find webpage */
        page = webkit_web_extension_get_page (web_ext, chmsg.pageid);
        verbose_s("_8");
        if (page != NULL)
          verbose_s ("_9");
        else
          verbose_s ("_a");

        if ( page != NULL )
            /* process the message */
            webext_execute_token (page, chmsg.token, chmsg.msg);
        else
            error ("cannot find page: %llu", chmsg.pageid);

        /* free the buffer */
        free (chmsg.msg);
    }
    /* TRUE means don't remove the event source */
    return TRUE;
}

G_MODULE_EXPORT void
webkit_web_extension_initialize_with_user_data (WebKitWebExtension *extension, const GVariant *data)
{
    int socket;

    /* initialize */
    web_ext = extension;

    g_variant_get ((GVariant *) data, G_GINT32_FORMAT, &socket);
    channel_init (socket, webext_message_read);
}
