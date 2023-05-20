/* See LICENSE file for copyright and license details. */

#include <sys/stat.h>
#include <pwd.h>

#include "file.h"
#include "sitespecific.h"
#include "surf.h"
#include "verbose.h"


static gchar *style_file = NULL;
gchar *file_script = NULL;
gchar *file_cookie = NULL;
gchar *dir_cache = NULL;


gboolean
files_init (void)
{
    const char *home_dir;
    gchar *config_dir;

    /* init */
    home_dir = dir_get_home ();  /* ~ */
    if ( home_dir == NULL )
        return FALSE;
    
    /* directories */
    dir_cache = g_build_filename (home_dir, ".cache", prog_name, NULL);  /* ~/.cache/surf */
    if ( !dir_check (dir_cache, FALSE) )
        return FALSE;

    config_dir = g_build_filename (home_dir, ".config", prog_name, NULL);  /* ~/.config/surf */
    if ( !dir_check (config_dir, FALSE) )
        goto quit;

    /* files */
    file_cookie = g_build_filename (dir_cache, "cookies", NULL);  /* ~/.cache/surf/cookies */
    if ( !file_check (file_cookie) )
        goto quit;

    file_script = g_build_filename (config_dir, "script.js", NULL);  /* ~/.config/surf/script.js */
    file_check (file_script);

    /* styles */
    if ( surf_style_file != NULL ) {
        style_file = dir_untilde (surf_style_file);
        if ( style_file == NULL || !file_check (style_file) )
            goto quit;
    } else if ( !styles_init (config_dir) )
        goto quit;
               
    /* certs */
    if ( !certs_init (config_dir) )
        goto quit;
    
    g_free (config_dir);
    return TRUE;

quit:

    g_free (config_dir);
    return FALSE;
}

gboolean
file_check (const char *path)
{
    FILE *f;

    f = fopen (path, "a");
    if ( f == NULL ) {
        error ("could not open file: %s", path);
        return FALSE;
    }

    chmod (path, 0600); /* always */
    fclose (f);
    return TRUE;
}

char *
dir_untilde (const char *path)
{
    const char *home_dir;
    const char *p = path;

    if ( *p == '~' && *++p == '/' ) {  /* incl. '\0' */
        home_dir = dir_get_home ();
        if ( home_dir == NULL )
            return NULL;

        return g_build_filename (home_dir, p + 1, NULL);
    }

    return g_strdup (path);
}

const char *
dir_get_home (void)
{
    const char *env;
    struct passwd *pw;

    env = getenv("HOME");
    if (env)
        return env;

    env = getenv("USER");
    if (env)
        pw = getpwnam(env);
    else
        pw = getpwuid(getuid());

    if ( pw == NULL ) {
        error ("can't get current user home directory");
        return NULL;
    }

    return pw->pw_dir;
}

gboolean
dir_check (const char *path, gboolean parents)
{
    char ret;
    struct stat st;

    if ( stat (path, &st) == 0 ) {
        if ( S_ISDIR (st.st_mode) )
            return TRUE;

        error ("%s is not a directory", path);
        return FALSE;
    }

    /* create new directory */
    if ( parents )
        ret = g_mkdir_with_parents (path, 0700);
    else
        ret = mkdir (path, 0700);

    if ( ret == -1 ) {
        error ("could not access directory: %s", path);
        return FALSE;
    }

    return TRUE;
}

void
files_free (void)
{
    /* styles and certs */
    sitespecific_free ();

    /* files */
    g_free (file_cookie);
    g_free (file_script);

    /* dirs */    
    g_free (dir_cache);
}
