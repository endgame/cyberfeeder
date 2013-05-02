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
#include "pages.h"

#include "card.h"
#include "card_db.h"
#include "card_set.h"
#include "ui_helpers.h"

extern struct card_db *DB;

enum card_list_columns {
  COL_NAME,
  COL_FACTION,
  COL_TYPE,
  COL_SET,
  COL_NUMBER,
  COL_CARD_PTR,
  N_COLS
};

static GtkWidget* setup_card_text_pane(GtkTextBuffer **buffer) {
  GtkWidget *text_view = gtk_text_view_new();
  *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(text_view), 5);

  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  gtk_container_add(GTK_CONTAINER(frame), text_view);
  return frame;
}

static void cursor_changed(GtkTreeSelection *sel, gpointer user_data) {
  GtkTreeModel *model;
  GtkTreeIter iter;
  if (!gtk_tree_selection_get_selected(sel, &model, &iter)) return;

  struct card *card;
  gtk_tree_model_get(model, &iter, COL_CARD_PTR, (gpointer)&card, -1);
  GtkTextBuffer *buffer = user_data;
  GtkTextIter start, end;
  gtk_text_buffer_get_start_iter(buffer, &start);
  gtk_text_buffer_get_end_iter(buffer, &end);
  gtk_text_buffer_delete(buffer, &start, &end);
  ui_helpers_text_buffer_add_card(buffer, &end, card);
}

static GtkWidget* setup_card_list_pane(GtkTextBuffer *text_buffer) {
  GtkListStore *store = gtk_list_store_new(N_COLS,
                                           G_TYPE_STRING, /* name */
                                           G_TYPE_STRING, /* faction */
                                           G_TYPE_STRING, /* type */
                                           G_TYPE_STRING, /* set */
                                           G_TYPE_INT,    /* number */
                                           G_TYPE_POINTER /* struct card* */);
  /* Build the list store. */
  for (guint i = 0; i < DB->sets->len; i++) {
    struct card_set *set = g_ptr_array_index(DB->sets, i);
    for (guint j = 0; j < set->cards->len; j++) {
      struct card *card = g_ptr_array_index(set->cards, j);
      GtkTreeIter iter;
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter,
                         COL_NAME, card->name,
                         COL_FACTION, faction_to_string(card->faction),
                         COL_TYPE, card->type_str,
                         COL_SET, card->set,
                         COL_NUMBER, card->number,
                         COL_CARD_PTR, card,
                         -1);
    }
  }

  /* Build the actual tree view. */
  GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
  GtkTreeViewColumn *column;
  static const struct {
    const char *name; gint column;
  } *p, table[] = {
    { "Name"   , COL_NAME    },
    { "Faction", COL_FACTION },
    { "Type"   , COL_TYPE    },
    { "Set"    , COL_SET     },
    { "Number" , COL_NUMBER  },
    { NULL     , -1          }
  };
  for (p = table; p->name != NULL; p++) {
    column =
      gtk_tree_view_column_new_with_attributes(p->name,
                                               gtk_cell_renderer_text_new(),
                                               "text", p->column,
                                               NULL);
    gtk_tree_view_column_set_sort_column_id(column, p->column);
    if (p->column == COL_NAME
        || p->column == COL_TYPE
        || p->column == COL_SET) {
      gtk_tree_view_column_set_resizable(column, TRUE);
    }
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
  }

  GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
  gtk_tree_selection_set_mode(sel, GTK_SELECTION_BROWSE);
  g_signal_connect_object(sel,
                          "changed",
                          G_CALLBACK(cursor_changed),
                          text_buffer,
                          0);
  GtkTreeIter iter;
  gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
  gtk_tree_selection_select_iter(sel, &iter);

  GtkWidget *scroller = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scroller), tree_view);

  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  gtk_container_add(GTK_CONTAINER(frame), scroller);
  return frame;
}

GtkWidget* the_toolbox(void) {
  GtkTextBuffer *text_buffer;
  GtkWidget *text_pane = setup_card_text_pane(&text_buffer);
  gtk_widget_set_size_request(text_pane, 250, -1);
  GtkWidget *list_pane = setup_card_list_pane(text_buffer);
  gtk_widget_set_size_request(list_pane, 400, -1);
  GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_paned_pack1(GTK_PANED(paned), list_pane, TRUE, FALSE);
  gtk_paned_pack2(GTK_PANED(paned), text_pane, FALSE, FALSE);
  return paned;
}
