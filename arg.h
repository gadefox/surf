/* See LICENSE file for copyright and license details. */

#ifndef _ARG_H_
#define _ARG_H_

#include "base.h"


typedef enum {
    VerboseKeys        = (1 << 0),
    VerboseButtons     = (1 << 1),
    VerboseToggleStats = (1 << 2),
    VerbosePageStats   = (1 << 3),
    VerboseAll         = VerboseKeys | VerboseButtons | VerboseToggleStats | VerbosePageStats
} VerboseInfo;

/* n/a ~ not used
 * i ~ command is sent from surf.rc to web browser
 * o ~ command is sent from web browser to surf.rc */
typedef enum {
    CommandUndefined,          /* n/a */
    /* Reserved for webkit notifications */
    CommandPageLoaded = 1,     /* o */
    /* User */
    CommandGo = 80,            /* i/o */
    CommandDownload = 81,      /* o */
    CommandFindInPage = 82,    /* i/o */
    CommandSearchEngine = 83,  /* i/o */
    CommandPlayVideo = 84,     /* o */
    CommandUserPassword = 85,  /* i/o */
    CommandPlumb = 86          /* o */
} CommandType;

typedef enum {
    AccessMicrophone,
    AccessWebcam,
    CaretBrowsing,
    Certificate,
    CookiePolicies,
    DiskCache,
    DefaultCharset,
    DNSPrefetch,
    Ephemeral,
    FileURLsCrossAccess,
    FontSize,
    Geolocation,
    HideBackground,
    HWAcceleration,
    Inspector,
    JavaScript,
    KioskMode,
    LoadImages,
    MediaManualPlay,
    Notifications,
    PreferredLanguages,
    RunInFullscreen,
    ScrollBars,
    ShowIndicators,
    SiteQuirks,
    SmoothScrolling,
    SpellChecking,
    SpellLanguages,
    StrictTLS,
    Style,
    WebGL,
    ZoomLevel,
    ParameterLast
} ParameterType;

/* Helper macros */
#define AccessMicrophone_0     AccessMicrophone
#define AccessWebcam_1         AccessWebcam
#define CaretBrowsing_2        CaretBrowsing
#define Certificate_3          Certificate
#define CookiePolicies_4       CookiePolicies
#define DiskCache_5            DiskCache
#define DefaultCharset_6       DefaultCharset
#define DNSPrefetch_7          DNSPrefetch
#define Ephemeral_8            Ephemeral
#define FileURLsCrossAccess_9  FileURLsCrossAccess
#define FontSize_10            FontSize
#define Geolocation_11         Geolocation
#define HideBackground_12      HideBackground
#define HWAcceleration_13      HWAcceleration
#define Inspector_14           Inspector
#define JavaScript_15          JavaScript
#define KioskMode_16           KioskMode
#define LoadImages_17          LoadImages
#define MediaManualPlay_18     MediaManualPlay
#define Notifications_19       Notifications
#define PreferredLanguages_20  PreferredLanguages
#define RunInFullscreen_21     RunInFullscreen
#define ScrollBars_22          ScrollBars
#define ShowIndicators_23      ShowIndicators
#define SiteQuirks_24          SiteQuirks
#define SmoothScrolling_25     SmoothScrolling
#define SpellChecking_26       SpellChecking
#define SpellLanguages_27      SpellLanguages
#define StrictTLS_28           StrictTLS
#define Style_29               Style
#define WebGL_30               WebGL
#define ZoomLevel_31           ZoomLevel


/* ATTN: the union must contain only "simple" types which
 * you would use as a parameter e.g. int, char, float,
 * double, pointers, enums etc., so no structures et al. */
typedef union {
    gboolean b;
    int i;
    uint u;
    float f;
    char *s;
    char **v;
    ParameterType p;
    CommandType c;
    VerboseInfo vi;
} Arg;


#endif /* _ARG_H_ */
