/* See LICENSE file for copyright and license details. */

#include "sitespecific.h"
#include "client.h"
#include "file.h"
#include "surf.h"
#include "verbose.h"
#include "webkit.h"


/* styles: The iteration will stop at the first match, beginning at the beginning of the list. */
static SiteSpecific styles [] = {
   /* regexp, file in $styledir, path is built in styles_init fn */
//    { ".*",   "default.css",     NULL }
};

/* certificates: Provide custom certificate for urls. */
static SiteSpecific certs [] = {
   /* regexp                file in $certdir    path is built in certs_init fn */
//    { "://suckless\\.org/", "suckless.org.crt", NULL }
};


int
reg_compile (regex_t *regex, const char *pattern)
{
    int ret;
    
    ret = regcomp (regex, pattern, REG_EXTENDED);
    if ( ret != 0 )
        warn ("could not compile regex: %s", pattern);

    return ret;
}

void
sitespecific_build_paths (SiteSpecific *ss, uint size, const char *dir)
{
    while ( size-- != 0 ) {
        if ( reg_compile (&(ss->re), ss->regex) != 0 ) {
            ss->regex = NULL;
            continue;
        }
        ss->path = g_build_filename (dir, ss->file, NULL);
        ss++;
    }
}

void
sitespecific_free_paths (SiteSpecific *ss, uint size)
{
    while ( size-- != 0 ) {
        g_free (ss->path);
        ss++;
    }
}

const gchar *
sitespecific_get_path (SiteSpecific *ss, uint size, const char *uri)
{
    while ( size-- != 0 ) {
        if ( ss->regex != NULL &&
             regexec (&(ss->re), uri, 0, NULL, 0) == 0 )
            return ss->path;
        ss++;
    }
    return NULL;
}

gboolean
styles_init (const gchar *config_dir)
{
    gchar *styles_dir;

    /* styles */
    if ( countof (styles) == 0 )  /* we don't want to create an empty folder */
        return TRUE;
        
    styles_dir = g_build_filename (config_dir, "styles", NULL);  /* ~/.config/surf/styles */
    if ( !dir_check (styles_dir, FALSE) ) {
        g_free (styles_dir);
        return FALSE;
    }
    
    sitespecific_build_paths (styles, countof (styles), styles_dir);

    g_free (styles_dir);
    return TRUE;
}

gboolean
certs_init (const gchar *config_dir)
{
    gchar *certs_dir;

    /* certs */
    if ( countof (certs) == 0 )  /* we don't want to create an empty folder */
        return TRUE;
        
    certs_dir = g_build_filename (config_dir, "certificates", NULL);  /* ~/.config/surf/certificates */
    if ( !dir_check (certs_dir, FALSE) ) {
        g_free (certs_dir);
        return FALSE;
    }

    sitespecific_build_paths (certs, countof (certs), certs_dir);

    g_free (certs_dir);
    return TRUE;
}

void
cert_set (Client *c, const char *uri)
{
    const gchar *file;
    char *host;
    GTlsCertificate *cert;
    WebKitWebContext *webcontext;
    
    file = sitespecific_get_path (certs, countof (certs), uri);
    if ( file == NULL )
        return;

    cert = g_tls_certificate_new_from_file (file, NULL);
    if ( cert == NULL ) {
        warn ("could not read certificate file: %s", file);
        return;
    }

    uri = strstr (uri, wk_prefix_https);
    if ( uri != NULL ) {
        uri += sizeof ("https://") - 1;

        host = strchr (uri, '/');
        if ( host != NULL ) {
            host = g_strndup (uri, host - uri);
            webcontext = webkit_web_view_get_context (c->view);
            webkit_web_context_allow_tls_certificate_for_host (webcontext, cert, host);
            g_free (host);
        }
    }
    g_object_unref (cert);
}

void
style_set_file (Client *c, const char *file)
{
    gchar *style;
    WebKitUserContentManager *contentmngr;
    WebKitUserStyleSheet *stylesheet;

    if ( !g_file_get_contents (file, &style, NULL, NULL) ) {
        warn ("could not read style file: %s", file);
        return;
    }

    contentmngr = webkit_web_view_get_user_content_manager (c->view);
    stylesheet = webkit_user_style_sheet_new (style, WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
          WEBKIT_USER_STYLE_LEVEL_USER, NULL, NULL);

    webkit_user_content_manager_add_style_sheet (contentmngr, stylesheet);

    g_free(style);
}

void
sitespecific_free (void)
{
    /* styles */
    g_free (surf_style_file);
    sitespecific_free_paths (styles, countof (styles));

    /* certs */
    sitespecific_free_paths (certs, countof (certs));
}

void
style_set (Client *c)
{
    const char *tmp;

    if ( surf_style_file != NULL )
        style_set_file (c, surf_style_file );
    else {
        tmp = webkit_web_view_get_uri (c->view);
        tmp = sitespecific_get_path (styles, countof (styles), tmp);
        if ( tmp != NULL )
            style_set_file (c, tmp );
    }
}
