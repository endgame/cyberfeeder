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

#include <gtk/gtk.h>
#include <stdlib.h>

#include "card_db.h"
#include "card_set.h"
#include "pages.h"

struct card_db *DB = NULL;
gchar *DATA_DIR = NULL;

static void read_db(void) {
  gchar *card_dir = g_strconcat(DATA_DIR, G_DIR_SEPARATOR_S, "cards", NULL);
  // TODO: Error checking!
  GDir *dir = g_dir_open(card_dir, 0, NULL);
  g_assert(dir != NULL);

  GPtrArray /* of gchar* */ *files = g_ptr_array_new_with_free_func(g_free);
  for (const gchar *file = g_dir_read_name(dir);
       file != NULL;
       file = g_dir_read_name(dir)) {
    if (g_str_has_suffix(file, ".json")) {
      g_ptr_array_add(files, g_strdup(file));
    }
  }
  g_dir_close(dir);

  gint cmp(gconstpointer a, gconstpointer b) {
    return strcmp(*(gchar**)a, *(gchar**)b);
  }
  g_ptr_array_sort(files, cmp);

  DB = card_db_new();
  for (guint i = 0; i < files->len; i++) {
    gchar *file = g_strconcat(card_dir,
                              G_DIR_SEPARATOR_S,
                              g_ptr_array_index(files, i),
                              NULL);
    // TODO: Error checking!
    struct card_set *set = card_set_load_file(file);
    g_assert(set != NULL);
    card_db_add_set(DB, set);
    g_free(file);
  }
  g_ptr_array_free(files, TRUE);
  g_free(card_dir);
}

int main(int argc, char *argv[]) {
  GOptionEntry options[] = {
    { "data-dir", 0, 0, G_OPTION_ARG_STRING, &DATA_DIR,
      "Location of data files", "DIR" },
    { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
  };

  gboolean ok = gtk_init_with_args(&argc, &argv, NULL, options, NULL, NULL);
  if (!ok) {
    fprintf(stderr, "Failed to initialise GTK.\n");
    exit(EXIT_FAILURE);
  }

  if (DATA_DIR == NULL) DATA_DIR = g_strdup(DEFAULT_DATA_DIR);

  read_db();

  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Cyberfeeder");
  gtk_window_set_default_size(GTK_WINDOW(window), 700, 300);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  GtkWidget *notebook = gtk_notebook_new();
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                           the_toolbox(), gtk_label_new("The Toolbox"));
  gtk_container_add(GTK_CONTAINER(window), notebook);
  gtk_widget_show_all(window);
  gtk_main();
  card_db_free(DB);
  DB = NULL;
  g_free(DATA_DIR);
  DATA_DIR = NULL;
  return 0;
}
