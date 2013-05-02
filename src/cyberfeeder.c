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

#include "card_db.h"
#include "card_set.h"
#include "pages.h"

struct card_db *DB = NULL;

int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);

  DB = card_db_new();
  struct card_set *set = card_set_load_file("data/00_Core.json");
  card_db_add_set(DB, set);

  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Cyberfeeder");
  gtk_window_set_default_size(GTK_WINDOW(window), 700, 300);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  gtk_container_add(GTK_CONTAINER(window), the_toolbox());
  gtk_widget_show_all(window);
  gtk_main();
  card_set_free(set);
  return 0;
}
