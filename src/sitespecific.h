/* See LICENSE file for copyright and license details. */

#ifndef _SITE_SPECIFIC_H_
#define _SITE_SPECIFIC_H_

#include <regex.h>
#include <glib.h>

#include "base.h"
#include "callback.h"


typedef struct {
    char *regex;
    char *file;
    gchar *path;  /* used due to g_free */
    regex_t re;
} SiteSpecific;


int reg_compile (regex_t *regex, const char *pattern);

void sitespecific_build_paths (SiteSpecific *ss, uint size, const char *dir);
void sitespecific_free_paths (SiteSpecific *ss, uint size);
const gchar * sitespecific_get_path (SiteSpecific *ss, uint size, const char *uri);

gboolean styles_init (const gchar *config_dir);
gboolean certs_init (const gchar *config_dir);
void sitespecific_free (void);

void cert_set (Client *c, const char *uri);
void style_set_file (Client *c, const char *file);
void style_set (Client *c);


#endif /* _SITE_SPECIFIC_H_ */
