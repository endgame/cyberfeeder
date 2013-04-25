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
#include "ui_helpers.h"

#include <string.h>

#include "card.h"

void ui_helpers_text_buffer_add_card(GtkTextBuffer *buffer,
                                     GtkTextIter *iter,
                                     const struct card *card) {
  g_assert(buffer != NULL);
  g_assert(card != NULL);

  gchar *header = g_strdup_printf("%s%s (%s #%d)\n"
                                  "%s %s\n",
                                  card->unique ? "\u25c6" : "",
                                  card->name,
                                  card->set,
                                  card->number,
                                  faction_to_string(card->faction),
                                  card->type_str);
  gtk_text_buffer_insert(buffer, iter, header, -1);
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

  gtk_text_buffer_insert(buffer, iter, card->text, -1);
  gtk_text_buffer_insert(buffer, iter, "\n", -1);

  if (card->flavor != NULL) {
    gtk_text_buffer_insert(buffer, iter, card->flavor, -1);
    gtk_text_buffer_insert(buffer, iter, "\n", -1);
  }

  if (card->illustrator != NULL) {
    gchar *illustrator = g_strdup_printf("Illustrator: %s\n",
                                         card->illustrator);
    gtk_text_buffer_insert(buffer, iter, illustrator, -1);
    g_free(illustrator);
  }
}
