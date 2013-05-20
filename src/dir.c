/* cyberfeeder - An Android: Netrunner deckbuilder
 * Copyright (C) 2013  Jack Kelly <jack@jackkelly.name>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "dir.h"

#include <string.h>

#include "load_error.h"

GPtrArray* dir_list_json(const char *path) {
  GError *err = NULL;
  GDir *dir = g_dir_open(path, 0, &err);
  if (dir == NULL) {
    load_error("%s", err->message);
    g_error_free(err);
    return NULL;
  }

  GPtrArray *rv = g_ptr_array_new_with_free_func(g_free);
  for (const char *file = g_dir_read_name(dir);
       file != NULL;
       file = g_dir_read_name(dir)) {
    if (g_str_has_suffix(file, ".json")) g_ptr_array_add(rv, g_strdup(file));
  }
  g_dir_close(dir);

  gint cmp(gconstpointer a, gconstpointer b) {
    return strcmp(*(const gchar**)a, *(const gchar**)b);
  }
  g_ptr_array_sort(rv, cmp);

  return rv;
}
