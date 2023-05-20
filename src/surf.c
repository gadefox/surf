/* See LICENSE file for copyright and license details. */

#include <sys/wait.h>  /* waitpid */

#include "surf.h"
#include "client.h"
#include "file.h"
#include "webkit.h"
#include "verbose.h"
#include "btn.h"
#include "key.h"


/* use main(int argc, char *argv []) */
#define ARGBEGIN \
for (surf_argv0 = *argv, argv++, argc--;\
     argv [0] && argv [0][0] == '-' && argv [0][1];\
     argc--, argv++) {\
    char argc_;\
    char **argv_;\
    int brk_;\
    if (argv [0][1] == '-' && argv [0][2] == '\0') {\
        argv++;\
        argc--;\
        break;\
    }\
    for (brk_ = 0, argv [0]++, argv_ = argv;\
        argv [0][0] && !brk_;\
        argv [0]++) {\
        if (argv_ != argv)\
            break;\
        argc_ = argv [0][0];\
        switch (argc_)

#define ARGEND    } }
#define ARGC()    argc_
#define EARGF(x)  (argv [0][1] == '\0' && argv [1] == NULL) ?\
    ((x), abort(), (char *) 0) : (brk_ = 1, (argv [0][1] != '\0') ?\
    (&argv [0][1]) : (argc--, argv++, argv [0]))

#define ARGF()    ((argv [0][1] == '\0' && argv[1] == NULL)?\
    (char *) 0 : (brk_ = 1, (argv [0][1] != '\0') ? (&argv [0][1]) : (argc--, argv++, argv [0])))


/* Appends `surf [version]' to default WebKit user agent.. (true) */
int surf_is_user_agent = TRUE;
/* ..or overrides the whole user agent string (not null) */
char *surf_full_user_agent = NULL;

/* Single file is used instead of files in $styledir */
char *surf_style_file = NULL;
char *surf_uri_default = "duckduckgo.com";

/* Webkit default features.  Highest priority value will be used.
 * Default parameters are priority  PriorityDefault
 * Per-uri parameters are priority  PriorityPerUri
 * Command parameters are priority  PriorityCommand */
Parameter surf_config [ParameterLast] = {
/* parameter                     Arg           Priority */
/* AccessMicrophone_0 */     { { .b = FALSE },  PriorityDefault },
/* AccessWebcam_1 */         { { .b = FALSE },  PriorityDefault },
/* CaretBrowsing_2 */        { { .b = FALSE },  PriorityDefault },
/* Certificate_3 */          { { .b = FALSE },  PriorityDefault },
/* CookiePolicies_4 */       { { .s = "@Aa" },  PriorityDefault },
/* DiskCache_5 */            { { .b = TRUE },   PriorityDefault },
/* DefaultCharset_6 */       { { .s = "UTF-8" },  PriorityDefault },
/* DNSPrefetch_7 */          { { .b = FALSE },  PriorityDefault },
/* Ephemeral_8 */            { { .b = FALSE },  PriorityDefault },
/* FileURLsCrossAccess_9 */  { { .b = FALSE },  PriorityDefault },
/* FontSize_10 */            { { .i = 12 },     PriorityDefault },
/* Geolocation_11 */         { { .b = FALSE },  PriorityDefault },
/* HideBackground_12 */      { { .b = FALSE },  PriorityDefault },
/* HWAcceleration_13 */      { { .u = WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS }, PriorityDefault },
/* Inspector_14 */           { { .b = FALSE },  PriorityDefault },
/* JavaScript_15 */          { { .b = TRUE },   PriorityDefault },
/* KioskMode_16 */           { { .b = FALSE },  PriorityDefault },
/* LoadImages_17 */          { { .b = TRUE },   PriorityDefault },
/* MediaManualPlay_18 */     { { .b = TRUE },   PriorityDefault },
/* Notifications_19 */       { { .b = FALSE },   PriorityDefault },
/* PreferredLanguages_20 */  { { .v = (char * []){ NULL } },  PriorityDefault },
/* RunInFullscreen_21 */     { { .b = FALSE },  PriorityDefault },
/* ScrollBars_22 */          { { .b = TRUE },   PriorityDefault },
/* ShowIndicators_23 */      { { .b = TRUE },   PriorityDefault },
/* SiteQuirks_24 */          { { .b = TRUE },   PriorityDefault },
/* SmoothScrolling_25 */     { { .b = FALSE },  PriorityDefault },
/* SpellChecking_26 */       { { .b = FALSE },  PriorityDefault },
/* SpellLanguages_27 */      { { .v = (char * []){ "en_US", NULL } },  PriorityDefault },
/* StrictTLS_28 */           { { .b = TRUE },   PriorityDefault },
/* Style_29 */               { { .b = TRUE },   PriorityDefault },
/* WebGL_30 */               { { .b = FALSE },  PriorityDefault },
/* ZoomLevel_31 */           { { .f = 1.0 },    PriorityDefault }
};

char *surf_argv0;


static void
usage (void)
{
    verbose_prefix ("usage");
    verbose_color (prog_name, VerboseWhite);
    verbose_c (' ');
    verbose_s ("[-AbBdDfFgGiIkKmLlMnNpPsStTvwxX] [-a cookiepolicies] [-c cookiefile] [-C stylefile] [-e xid] [-H historyfile] [-r scriptfile] [-u useragent] [-z zoomlevel] [uri]\n");
    verbose_color ("see LICENSE for © details\n", VerboseWhite);

    /* print shortcut keys and button event */
    verbose_newline ();
    keys_verbose ();
    verbose_newline ();
    btns_verbose ();

    /* die */
    exit (EXIT_SUCCESS);
}

static void
version (void)
{
    verbose_s (VERSION);
    verbose_newline ();

    /* die */
    exit (EXIT_SUCCESS);
}

static void 
surf_sigchld (int unused)
{
    pid_t pid;
    int status;

    /* Exterminate */
    while ( (pid = waitpid (-1, &status, WNOHANG)) > 0 )
        info ("child %ld terminated", pid);
}

static void
surf_sighup (int unused)
{
    clients_reload (FALSE);
}

static int
surf_run (const char *uri)
{
    Client *c;
    struct sigaction sa;

    /* init modules */
    if ( !files_init () )
        return EXIT_FAILURE;

    if ( !clients_init () )
        return EXIT_FAILURE;

    parameters_init ();
    webkit_init ();
    key_init ();

    /* clean up any zombies immediately */
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = surf_sigchld;

    if ( sigaction (SIGCHLD, &sa, NULL) == -1 ) {
        error ("can't install SIGCHLD handler");
        return EXIT_FAILURE;
    }

    if ( signal (SIGHUP, surf_sighup) == SIG_ERR ) {
        error ("can't install SIGHUP handler");
        return EXIT_FAILURE;
    }

    /* create new client with view */
    c = clients_prepend ();
    if ( c == NULL )
        return EXIT_FAILURE;

    client_view_show (c);
    client_uri_load (c, uri);

#ifdef FEATURE_TITLE    
    client_update_title (c);
#endif

    gtk_main ();
    return EXIT_SUCCESS;
}

void
surf_cleanup (void)
{
    /* free modules */
    clients_free ();
    files_free ();
    webkit_free ();

    /* program name */
    g_free ((gchar *) prog_name);
}

int
main (int argc, char **argv)
{
    int ret;

    /* initialize program name */
    prog_name = g_path_get_basename (*argv);

    /* command line args */
    ARGBEGIN {
    case 'a':
        surf_config [CookiePolicies].val.s = EARGF(usage());
        surf_config [CookiePolicies].prio = PriorityCommand;
        break;
    case 'A':
        surf_config [HWAcceleration].val.u = WEBKIT_HARDWARE_ACCELERATION_POLICY_NEVER;
        surf_config [HWAcceleration].prio = PriorityCommand;
        break;
    case 'b':
        surf_config [ScrollBars].val.b = FALSE;
        surf_config [ScrollBars].prio = PriorityCommand;
        break;
    case 'B':
        surf_config [ScrollBars].val.b = TRUE;
        surf_config [ScrollBars].prio = PriorityCommand;
        break;
    case 'c':
        file_cookie = EARGF(usage());
        break;
    case 'C':
        surf_style_file = EARGF(usage());
        break;
    case 'd':
        surf_config [DiskCache].val.b = FALSE;
        surf_config [DiskCache].prio = PriorityCommand;
        break;
    case 'D':
        surf_config [DiskCache].val.b = TRUE;
        surf_config [DiskCache].prio = PriorityCommand;
        break;
    case 'e':
        wk_embed = strtol(EARGF(usage()), NULL, 0);
        break;
    case 'f':
        surf_config [RunInFullscreen].val.b = FALSE;
        surf_config [RunInFullscreen].prio = PriorityCommand;
        break;
    case 'F':
        surf_config [RunInFullscreen].val.b = TRUE;
        surf_config [RunInFullscreen].prio = PriorityCommand;
        break;
    case 'g':
        surf_config [Geolocation].val.b = FALSE;
        surf_config [Geolocation].prio = PriorityCommand;
        break;
    case 'G':
        surf_config [Geolocation].val.b = TRUE;
        surf_config [Geolocation].prio = PriorityCommand;
        break;
    case 'i':
        surf_config [LoadImages].val.b = FALSE;
        surf_config [LoadImages].prio = PriorityCommand;
        break;
    case 'I':
        surf_config [LoadImages].val.b = TRUE;
        surf_config [LoadImages].prio = PriorityCommand;
        break;
    case 'k':
        surf_config [KioskMode].val.b = FALSE;
        surf_config [KioskMode].prio = PriorityCommand;
        break;
    case 'K':
        surf_config [KioskMode].val.b = TRUE;
        surf_config [KioskMode].prio = PriorityCommand;
        break;
    case 'l':
        surf_config [Notifications].val.b = FALSE;
        surf_config [Notifications].prio = PriorityCommand;
        break;
    case 'L':
        surf_config [Notifications].val.b = TRUE;
        surf_config [Notifications].prio = PriorityCommand;
        break;
    case 'm':
        surf_config [Style].val.b = FALSE;
        surf_config [Style].prio = PriorityCommand;
        break;
    case 'M':
        surf_config [Style].val.b = TRUE;
        surf_config [Style].prio = PriorityCommand;
        break;
    case 'n':
        surf_config [Inspector].val.b = FALSE;
        surf_config [Inspector].prio = PriorityCommand;
        break;
    case 'N':
        surf_config [Inspector].val.b = TRUE;
        surf_config [Inspector].prio = PriorityCommand;
        break;
    case 'r':
        file_script = EARGF(usage());
        break;
    case 's':
        surf_config [JavaScript].val.b = FALSE;
        surf_config [JavaScript].prio = PriorityCommand;
        break;
    case 'S':
        surf_config [JavaScript].val.b = TRUE;
        surf_config [JavaScript].prio = PriorityCommand;
        break;
    case 't':
        surf_config [StrictTLS].val.b = FALSE;
        surf_config [StrictTLS].prio = PriorityCommand;
        break;
    case 'T':
        surf_config [StrictTLS].val.b = TRUE;
        surf_config [StrictTLS].prio = PriorityCommand;
        break;
    case 'u':
        surf_full_user_agent = EARGF(usage());
        break;
    case 'v':
        version ();
        /* die */
    case 'w':
        wk_show_xid = TRUE;
        break;
    case 'x':
        surf_config [Certificate].val.b = FALSE;
        surf_config [Certificate].prio = PriorityCommand;
        break;
    case 'X':
        surf_config [Certificate].val.b = TRUE;
        surf_config [Certificate].prio = PriorityCommand;
        break;
    case 'z':
        surf_config [ZoomLevel].val.f = strtof(EARGF(usage()), NULL);
        surf_config [ZoomLevel].prio = PriorityCommand;
        break;
    default:
        usage();
    } ARGEND;

    /* initialize and run */
    ret = surf_run (argc != 0 ? *argv : surf_uri_default);
    surf_cleanup ();
    return ret;
}
