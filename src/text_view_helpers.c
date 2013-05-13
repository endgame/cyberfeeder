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
#include "text_view_helpers.h"

#include <string.h>

#include "card.h"

/* Call pango to parse its markup and insert it into a GtkTextBuffer. It
   is SHAMEFUL that GTK bug 59390 hasn't been fixed[1]. When it has,
   this can go away.

   Inspired by the neglected patch attached to said bug.

   [1]: https://bugzilla.gnome.org/show_bug.cgi?id=59390 */

void text_buffer_insert_markup(GtkTextBuffer *buffer,
                               GtkTextIter *iter,
                               const gchar *markup) {
  PangoAttrList *attr_list;
  gchar *text;
  // TODO: error detection and logging.
  gboolean ok =
    pango_parse_markup(markup, -1, 0, &attr_list, &text, NULL, NULL);
  g_assert(ok);

  if (attr_list == NULL) {
    gtk_text_buffer_insert(buffer, iter, markup, -1);
    g_free(text);
    return;
  }

  PangoAttrIterator *pa_iter = pango_attr_list_get_iterator(attr_list);
  do {
    PangoAttribute *attr;
    gint start, end;
    pango_attr_iterator_range(pa_iter, &start, &end);

    if (end == G_MAXINT) end = start - 1;

    GtkTextTag *tag = gtk_text_buffer_create_tag(buffer, NULL, NULL);
    if ((attr = pango_attr_iterator_get(pa_iter, PANGO_ATTR_LANGUAGE))) {
      g_object_set(tag, "language",
                   pango_language_to_string(((PangoAttrLanguage*)attr)->value),
                   NULL);
    }

    if ((attr = pango_attr_iterator_get(pa_iter, PANGO_ATTR_FAMILY))) {
      g_object_set(tag, "family", ((PangoAttrString*)attr)->value, NULL);
    }

    static const struct {
      const gchar* name; const PangoAttrType pango_name;
    } *p, table[] = {
      { "style"     , PANGO_ATTR_STYLE     },
      { "weight"    , PANGO_ATTR_WEIGHT    },
      { "variant"   , PANGO_ATTR_VARIANT   },
      { "stretch"   , PANGO_ATTR_STRETCH   },
      { "size"      , PANGO_ATTR_SIZE      },
      { "underline" , PANGO_ATTR_UNDERLINE },
      { "rise"      , PANGO_ATTR_RISE      },
      { NULL        , -1                   }
    };
    for (p = table; p->name != NULL; p++) {
      if ((attr = pango_attr_iterator_get(pa_iter, p->pango_name))) {
        g_object_set(tag, p->name, ((PangoAttrInt*)attr)->value, NULL);
      }
    }

    if ((attr = pango_attr_iterator_get(pa_iter, PANGO_ATTR_FONT_DESC))) {
      g_object_set(tag, "font-desc", ((PangoAttrFontDesc*)attr)->desc, NULL);
    }

    if ((attr = pango_attr_iterator_get(pa_iter, PANGO_ATTR_FOREGROUND))) {
      PangoAttrColor *c = (PangoAttrColor*)attr;
      GdkColor col = { 0, c->color.red, c->color.green, c->color.blue };
      g_object_set(tag, "foreground-gdk", &col, NULL);
    }
    if ((attr = pango_attr_iterator_get(pa_iter, PANGO_ATTR_BACKGROUND))) {
      PangoAttrColor *c = (PangoAttrColor*)attr;
      GdkColor col = { 0, c->color.red, c->color.green, c->color.blue };
      g_object_set(tag, "background-gdk", &col, NULL);
    }

    if ((attr = pango_attr_iterator_get(pa_iter, PANGO_ATTR_STRIKETHROUGH))) {
      g_object_set(tag, "strikethrough",
                   (gboolean)((PangoAttrInt*)attr)->value, NULL);
    }

    if ((attr = pango_attr_iterator_get(pa_iter, PANGO_ATTR_SCALE))) {
      g_object_set(tag, "scale", ((PangoAttrFloat*)attr)->value, NULL);
    }

    gtk_text_buffer_insert_with_tags(buffer, iter,
                                     text + start, end - start,
                                     tag, NULL);
  } while (pango_attr_iterator_next(pa_iter));

  pango_attr_iterator_destroy(pa_iter);
  pango_attr_list_unref(attr_list);
  g_free(text);
}

GtkTextView *text_view_make_uneditable(GtkTextView *text_view) {
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(text_view), 5);
  return text_view;
}

void text_buffer_add_card(GtkTextBuffer *buffer,
                          GtkTextIter *iter,
                          const struct card *card) {
  g_assert(buffer != NULL);
  g_assert(card != NULL);

  gchar *name = card_render_name(card);
  gchar *header = g_strdup_printf("<big>%s</big>\n"
                                  "<small>(%s #%d)</small>\n"
                                  "%s %s\n"
                                  "\n",
                                  name,
                                  card->set,
                                  card->number,
                                  faction_to_string(card->faction),
                                  card->type_str);
  g_free(name);
  text_buffer_insert_markup(buffer, iter, header);
  g_free(header);

  if (strcmp(card->set, "Core") == 0) {
    gchar *quantity = g_strdup_printf("Quantity: %d\n", card->quantity);
    gtk_text_buffer_insert(buffer, iter, quantity, -1);
    g_free(quantity);
  }

  if (card_has_cost(card)) {
    gchar *cost = g_strdup_printf("Cost: %s\n", card->cost);
    gtk_text_buffer_insert(buffer, iter, cost, -1);
    g_free(cost);
  }

  static const struct {
    const gchar* label;
    const goffset off;
    gboolean (*predicate)(const struct card*);
  } *p, fields[] = {
#define FIELD(L, X) { L, offsetof(struct card, X), card_has_ ## X }
    FIELD("Base Link", base_link),
    FIELD("Minimum Deck Size", min_decksize),
    FIELD("Maximum Influence", max_influence),
    FIELD("Agenda Points", agenda_points),
    FIELD("Influence Cost", influence_cost),
    FIELD("Trash Cost", trash_cost),
    FIELD("Memory Cost", memory_cost),
    FIELD("Strength", strength),
#undef FIELD
    { NULL, -1, NULL }
  };

  for (p = fields; p->label != NULL; p++) {
    if (!p->predicate(card)) continue;
    gchar *line = g_strdup_printf("%s: %d\n",
                                  p->label,
                                  *((gint8*)card + p->off));
    gtk_text_buffer_insert(buffer, iter, line, -1);
    g_free(line);
  }

  gtk_text_buffer_insert(buffer, iter, "\n", -1);
  text_buffer_insert_markup(buffer, iter, card->text);
  gtk_text_buffer_insert(buffer, iter, "\n", -1);

  if (card->flavor != NULL) {
    text_buffer_insert_markup(buffer, iter, card->flavor);
    gtk_text_buffer_insert(buffer, iter, "\n", -1);
  }

  if (card->illustrator != NULL) {
    gtk_text_buffer_insert(buffer, iter, "\n", -1);
    gchar *illustrator = g_strdup_printf("Illustrator: %s\n",
                                         card->illustrator);
    gtk_text_buffer_insert(buffer, iter, illustrator, -1);
    g_free(illustrator);
  }
}
