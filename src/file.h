/* See LICENSE file for copyright and license details. */

#ifndef _FILE_H_
#define _FILE_H_

#include <glib.h>


extern gchar *file_script;
extern gchar *file_cookie;
extern gchar *dir_cache;


gboolean files_init (void);
void files_free (void);

gboolean file_check (const char *path);
gchar * dir_untilde (const char *path);
gboolean dir_check (const char *path, gboolean parents);
const char *dir_get_home (void);


#endif /* _FILE_H_ */
