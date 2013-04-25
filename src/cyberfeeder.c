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

#include "card.h"
#include "card_set.h"
#include "ui_helpers.h"

enum columns {
  NAME,
  N_COLUMNS
};

int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);

  struct card_set *set = card_set_load_file("data/00_Core.json");
  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Cyberfeeder");
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  GtkListStore *store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING);
  for (guint i = 0; i < set->cards->len; i++) {
    struct card *card = g_ptr_array_index(set->cards, i);
    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, NAME, card->name, -1);
  }
  GtkWidget *card_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *column =
    gtk_tree_view_column_new_with_attributes ("Name", renderer, "markup",
                                              NAME, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(card_list), column);

  GtkWidget *scroller = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scroller), card_list);
  // gtk_container_add(GTK_CONTAINER(window), scroller);
  GtkWidget *textview = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  GtkTextIter it;
  gtk_text_buffer_get_end_iter(buffer, &it);
  ui_helpers_text_buffer_add_card(buffer, &it,
                                  g_ptr_array_index(set->cards, 98));
  gtk_container_add(GTK_CONTAINER(window), textview);
  gtk_widget_show_all(window);
  gtk_main();
  card_set_free(set);
  return 0;
}
