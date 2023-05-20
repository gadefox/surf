/* See LICENSE file for copyright and license details. */

#include <webkit2/webkit2.h>

#include "parameter.h"
#include "key.h"
#include "sitespecific.h"
#include "surf.h"
#include "webkit.h"


static ParameterType load_transient [] = {
    Certificate_3,
    CookiePolicies_4,
    DiskCache_5,
    DNSPrefetch_7,
    FileURLsCrossAccess_9,
    JavaScript_15,
    LoadImages_17,
    PreferredLanguages_20,
    ShowIndicators_23,
    StrictTLS_28
};

static ParameterType load_committed [] = {
//	AccessMicrophone_0,
//	AccessWebcam_1,
    CaretBrowsing_2,
    DefaultCharset_6,
    FontSize_10,
    Geolocation_11,
    HideBackground_12,
    Inspector_14,
//	KioskMode_16,
    MediaManualPlay_18,
    RunInFullscreen_21,
    ScrollBars_22,
    SiteQuirks_24,
    SmoothScrolling_25,
    SpellChecking_26,
    SpellLanguages_27,
    Style_29,
    ZoomLevel_31
};

static ParameterType load_finished [] = {
};

static UriParameters *cur_params = NULL;
static Priority mod_params [ParameterLast];


/* ATTN: Please note the array w/ NameParameters must be sorted by
 * ParamName so use macros w/ numbers instead of enumeration. */
static UriParameters uri_params [] = {
#if 0
/*    uri */
    { "(://|\\.)suckless\\.org(/|$)", (TypeParameter []){
/*         ParameterType         Arg           Priority */
         { JavaScript_15,    { { .b = FALSE }, PriorityPerUri } },
         { ParameterLast } }  /* Dummy item for array termination */
    }
#endif  /* 0 */
};

int param_cookie_policy;


void
parameters_init (void)
{
    uint i, cnt;
    UriParameters *up;
    Parameter *sparam;
    ParameterType t;
    TypeParameter *tp;

    /* init */
    param_cookie_policy = 0;

    /* uri params */
    for ( i = 0, up = uri_params; i < countof (uri_params); i++, up++ ) {
        if ( reg_compile (&(up->re), up->uri) != 0 ) {
            up->uri = NULL;
            continue;
        }

        /* copy default parameters with higher priority */
        for ( cnt = 0, tp = up->config, t = tp->type;
              t != ParameterLast;
              cnt++, tp++, t = tp->type ) {
            /* Please note t is in <0, ParameterLast) range */
            sparam = surf_config + t;  /* ~ &def_config [t] */
                                               
            /* FIXME: uri param should be above the default configuration when the priorities are equal. */
            if ( tp->param.prio > sparam->prio )
                continue;
                                                                    
            tp->param.prio = sparam->prio;
            tp->param.val = sparam->val;
        }
                                
        /* set counted size */
        up->size = cnt;
    }
}

void
parameter_set (Client *c, gboolean refresh, ParameterType type, const Arg a)
{
    GdkRGBA bgcolor = { 0 };
    WebKitSettings *settings;
    WebKitWebContext *webcontext;

    settings = webkit_web_view_get_settings (c->view);
    mod_params [type] = parameter_get_priority (type);

    switch (type) {
        case AccessMicrophone:
            /* do nothing */
            return;

        case AccessWebcam:
            /* do nothing */
            return;

        case CaretBrowsing:
            webkit_settings_set_enable_caret_browsing (settings, a.b);
            refresh = FALSE;
            break;

        case Certificate:
          {
            const char *uri;

            if (a.b) {
                uri = webkit_web_view_get_uri (c->view);
                cert_set (c, uri);
            }
            /* do not update */
            return;
          }

        case CookiePolicies:
          {
            WebKitCookieAcceptPolicy cookiepolicy;
            WebKitCookieManager *cookiemngr;

            webcontext = webkit_web_view_get_context (c->view);
            cookiemngr = webkit_web_context_get_cookie_manager (webcontext);
            cookiepolicy = webkit_cookie_policy_get (param_cookie_policy);
            webkit_cookie_manager_set_accept_policy (cookiemngr, cookiepolicy);

            refresh = FALSE;
            break;
          }

        case DiskCache:
            webcontext = webkit_web_view_get_context (c->view);

            webkit_web_context_set_cache_model (webcontext,
                   a.b ? WEBKIT_CACHE_MODEL_WEB_BROWSER : WEBKIT_CACHE_MODEL_DOCUMENT_VIEWER);
             /* do not update */
            return;

        case DefaultCharset:
            webkit_settings_set_default_charset (settings, a.s);
            /* do not update */
            return;

        case DNSPrefetch:
            webkit_settings_set_enable_dns_prefetching (settings, a.b);
            /* do not update */
            return;

        case FileURLsCrossAccess:
            webkit_settings_set_allow_file_access_from_file_urls (settings, a.b);
            webkit_settings_set_allow_universal_access_from_file_urls (settings, a.b);
            /* do not update */
            return;

        case FontSize:
            webkit_settings_set_default_font_size (settings, a.i);
            /* do not update */
            return;

        case Geolocation:
            refresh = FALSE;
            break;

        case HideBackground:
            if (a.b)
                webkit_web_view_set_background_color (c->view, &bgcolor);
            /* do not update */
            return;

        case HWAcceleration:
            webkit_settings_set_hardware_acceleration_policy (settings, a.u);
            /* do no update */
            return;

        case Inspector:
            webkit_settings_set_enable_developer_extras (settings, a.b);
            /* do not update */
            return;

        case JavaScript:
            webkit_settings_set_enable_javascript (settings, a.b);
            break;

        case KioskMode:
            /* do nothing */
            return;

        case LoadImages:
            webkit_settings_set_auto_load_images (settings, a.b);
            break;

        case MediaManualPlay:
            webkit_settings_set_media_playback_requires_user_gesture (settings, a.b);
            break;

        case Notifications:
            refresh = FALSE;
            break;

        case PreferredLanguages:
            /* do nothing */
            return;

        case RunInFullscreen:
            /* do nothing */
            return;

        case ScrollBars:
            /* do not update */
            return;

        case ShowIndicators:
            break;

        case SmoothScrolling:
            webkit_settings_set_enable_smooth_scrolling (settings, a.b);
            /* do not update */
            return;

        case SiteQuirks:
            webkit_settings_set_enable_site_specific_quirks (settings, a.b);
            break;

        case SpellChecking:
            webcontext = webkit_web_view_get_context (c->view);
            webkit_web_context_set_spell_checking_enabled (webcontext, a.b);
            /* do not update */
            return;

        case SpellLanguages:
            /* do nothing */
            return;

        case StrictTLS:
          {
              WebKitWebsiteDataManager *datamngr;

              datamngr = webkit_web_view_get_website_data_manager (c->view);
              webkit_website_data_manager_set_tls_errors_policy (datamngr, a.b ?
                WEBKIT_TLS_ERRORS_POLICY_FAIL : WEBKIT_TLS_ERRORS_POLICY_IGNORE);
            break;
          }

        case Style:
          {
            WebKitUserContentManager *contentmngr;

            contentmngr = webkit_web_view_get_user_content_manager (c->view);
            webkit_user_content_manager_remove_all_style_sheets (contentmngr);

            if ( a.b )
                style_set (c);
        
            refresh = FALSE;
            break;
          }

        case WebGL:
            webkit_settings_set_enable_webgl (settings, a.b);
            break;

        case ZoomLevel:
            webkit_web_view_set_zoom_level (c->view, a.f);
            /* do not update */
            return;

        default:
            /* do nothing */
            return;
    }

#ifdef FEATURE_TITLE
    client_update_title (c);
#endif

    if ( refresh )
        client_reload (c, FALSE);
}

UriParameters *
uriparameters_find (const char *uri)
{
    UriParameters *up;
    uint i;

    for ( i = 0, up = uri_params; i < countof (uri_params); i++, up++ ) {
        if ( up->uri == NULL )
            continue;

        if ( regexec (&(up->re), uri, 0, NULL, 0) == 0 )
            return up;
    }

    return NULL;
}

void
uriparameters_set (Client *c, const char *uri, ParameterType *params, uint size)
{
    Parameter *uparam, *sparam;
    ParameterType p;

    /* change current uri parameters */
    cur_params = uriparameters_find (uri);

    /* iteration: go through all params, check the priority and set the parameter */
    while ( size-- != 0 ) {
        p = *params++;
        sparam = surf_config + p;  /* ~ &surf_config [p] */

        /* Note uparam points to either cur_params (uri_params) when ParamName is defined or def_config */
        uparam = parameter_get (p);

        switch (p) {
            default:
                /* parameter_get function returns default configuration when cur_params is null or the ParamName $p is not included in the
                 * array. Then dparam equals to uparam which means dparam->prio equals to uparam->prio.
                 * FIXME Is the first condition necessary?? Please note we update uri parameters with prio less then dparam->prio (see setup function). */
                if ( sparam->prio >= uparam->prio &&
                     sparam->prio >= mod_params [p] )
                    continue;
            /* FALLTHROUGH */
            case Certificate:
            case CookiePolicies:
            case Style:
                parameter_set (c, FALSE, p, uparam->val);
                break;
        }
    }
}

/* ATTN: This implementation assumes the parameters are sorted by it's type */
Parameter *
uriparameter_find (ParameterType p)
{
    uint low, mid, high;
    TypeParameter *pivot;
    ParameterType type;
    TypeParameter *config;

    /* init margin and pointers */
    config = cur_params->config;
    low = 0;
    high = cur_params->size - 1;

    /* binary search */
    while ( low <= high ) {
        /* check middle item */
        mid = (low + high) >> 1;
        pivot = config + mid;
        
        type = pivot->type;
        if ( type < p )
            low = mid + 1;
        else if ( type > p )
            high = mid - 1;
        else
            return &pivot->param;
    }
    
    /* unsuccessful: the vector does not contain an entry with ParameterType */
    return NULL;
}

Parameter *
parameter_get (ParameterType p)
{
    Parameter *param;

    /* check current  uri parameters first and then the the default array */
    if ( cur_params != NULL ) {
        param = uriparameter_find (p);
        if ( param != NULL )
            return param;
    }

    return surf_config + p;  /* ~ &surf_config [p] */
}

void
uriparameters_transient_set (Client *c, const char *uri)
{
    uriparameters_set (c, uri, load_transient, countof (load_transient));
}

void
uriparameters_committed_set (Client *c, const char *uri)
{
    uriparameters_set (c, uri, load_committed, countof (load_committed));
}

void
uriparameters_finished_set (Client *c, const char *uri)
{
    uriparameters_set (c, uri, load_finished, countof (load_finished));
}

void parameter_toggle_cookie_policy (Client *c)
{
    char *cookie_policies;

    ++param_cookie_policy;
    cookie_policies = parameter_get_string (CookiePolicies);
    param_cookie_policy %= strlen (cookie_policies);

    parameter_set (c, FALSE, CookiePolicies, (Arg) 0);
}
