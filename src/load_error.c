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
#include "load_error.h"

#include <gtk/gtk.h>

#include "card.h"
#include "widget_helpers.h"

static GPtrArray /* of gchar*, owned */ *errors = NULL;
static GQueue /* of gchar*, owned */ sources = G_QUEUE_INIT;

static inline void setup(void) {
  if (errors == NULL) errors = g_ptr_array_new_with_free_func(g_free);
}

static gchar* build_source(void) {
  GString *str = g_string_new("");
  void f(gpointer source, gpointer str) {
    g_string_append(str, source);
  }
  g_queue_foreach(&sources, f, str);
  return g_string_free(str, FALSE);
}

void load_error_push_source(const char *format, ...) {
  g_assert(format != NULL);
  va_list args;
  va_start(args, format);
  g_queue_push_tail(&sources, g_strdup_vprintf(format, args));
  va_end(args);
}

void load_error_pop_source(void) {
  g_assert(!g_queue_is_empty(&sources));
  g_free(g_queue_pop_tail(&sources));
}

void load_error(const char *format, ...) {
  setup();
  gchar *src = build_source();
  va_list args;
  va_start(args, format);
  gchar *msg = g_strdup_vprintf(format, args);
  va_end(args);

  g_ptr_array_add(errors, g_strconcat(src, msg, NULL));
  g_free(src);
  g_free(msg);
}

void load_error_card(const struct card *card,
                     const char *format, ...) {
  setup();
  va_list args;
  va_start(args, format);
  gchar *msg = g_strdup_vprintf(format, args);
  va_end(args);

  g_ptr_array_add(errors, g_strconcat(card->name, ": ", msg, NULL));
  g_free(msg);
}

/* Show all errors in a dialog, then free allocated memory. */
void load_error_show(void) {
  if (errors == NULL) return;

  GtkWidget *dialog =
    gtk_dialog_new_with_buttons("Load Errors",
                                NULL,
                                GTK_DIALOG_MODAL,
                                GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                NULL);

  GtkListStore *store = gtk_list_store_new(1, G_TYPE_STRING);
  GtkTreeIter iter;
  for (guint i = 0; i < errors->len; i++) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, g_ptr_array_index(errors, i), -1);
  }
  GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
  g_object_unref(store);
  GtkTreeViewColumn *column =
    gtk_tree_view_column_new_with_attributes("Message",
                                             gtk_cell_renderer_text_new(),
                                             "text", 0,
                                             NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

  GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  GtkWidget *frame = widget_wrap_frame(widget_wrap_scroller(tree_view));
  gtk_box_pack_start(GTK_BOX(content), frame, TRUE, TRUE, 0);
  gtk_widget_set_size_request(content, 450, -1);

  gtk_widget_show_all(frame);
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);

  g_ptr_array_free(errors, TRUE);
  errors = NULL;
}
