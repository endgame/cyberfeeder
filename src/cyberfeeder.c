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
#include "dir.h"
#include "load_error.h"
#include "pages.h"

struct card_db *DB = NULL;
gchar *DATA_DIR = NULL;

static void read_db(void) {
  DB = card_db_new();

  gchar *card_dir = g_build_filename(DATA_DIR, "cards", NULL);
  void add_card_set(const char *file, gpointer db) {
    struct card_set *set = card_set_load_file(file);
    if (set != NULL) card_db_add_set(db, set);
  }
  dir_json_foreach(card_dir, add_card_set, DB);
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
  load_error_show();
  if (DB->sets->len == 0) {
    card_db_free(DB);
    DB = NULL;
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "Failed to load DB.");
    gtk_window_set_title(GTK_WINDOW(dialog), "Error");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    exit(EXIT_FAILURE);
  }

  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Cyberfeeder");
  gtk_window_set_default_size(GTK_WINDOW(window), 700, 300);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  GtkWidget *notebook = gtk_notebook_new();
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                           the_toolbox(), gtk_label_new("The Toolbox"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                           tmi(), gtk_label_new("TMI"));
  gtk_container_add(GTK_CONTAINER(window), notebook);
  gtk_widget_show_all(window);
  gtk_main();
  card_db_free(DB);
  DB = NULL;
  g_free(DATA_DIR);
  DATA_DIR = NULL;
  return 0;
}
