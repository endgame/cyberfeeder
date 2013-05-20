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
#include "card.h"

#include <errno.h>
#include <string.h>

#include "hash.h"
#include "load_error.h"

const gchar* faction_to_string(enum faction faction) {
  switch (faction) {
  case RUNNER_ANARCH:     return "Anarch";
  case RUNNER_CRIMINAL:   return "Criminal";
  case RUNNER_SHAPER:     return "Shaper";
  case RUNNER_NEUTRAL:    return "Neutral Runner";
  case CORP_HAAS_BIOROID: return "Haas-Bioroid";
  case CORP_JINTEKI:      return "Jinteki";
  case CORP_NBN:          return "NBN";
  case CORP_WEYLAND:      return "Weyland";
  case CORP_NEUTRAL:      return "Neutral Corp";
  default: g_assert_not_reached();
  }
}

struct card* card_new(enum faction faction,
                      const gchar *type,
                      const gchar *set,
                      gint8 number,
                      gint8 quantity,
                      gboolean unique,
                      const gchar *name,
                      const gchar *text,
                      const gchar *flavor,
                      const gchar *illustrator) {
  g_assert(type != NULL);
  g_assert(set != NULL);
  g_assert(number > 0);
  g_assert(quantity > 0);
  g_assert(name != NULL);
  g_assert(text != NULL);

  enum card_type c_type;
  gboolean ok = hash_card_type(type, faction, &c_type);
  if (!ok) {
    load_error(".type: Invalid card type \"%s\"", type);
    return NULL;
  }

  struct card *card = g_slice_new(struct card);
  card->faction = faction;
  card->type = c_type;
  card->type_str = g_strdup(type);
  card->set = set;
  card->number = number;
  card->quantity = quantity;
  card->unique = unique;
  card->name = g_strdup(name);
  card->text = g_strdup(text);
  card->flavor = g_strdup(flavor);
  card->illustrator = g_strdup(illustrator);

  card->cost = NULL;
  card->agenda_points = -1;
  card->influence_cost = card_is_neutral(card) ? 0 : -1;
  card->trash_cost = -1;
  card->memory_cost = -1;
  card->strength = -1;
  card->min_decksize = -1;
  card->max_influence = -1;
  card->base_link = -1;
  return card;
}

// TODO: log errors and such on fill funcs
static struct card* card_fill_id(struct card *card,
                                 gint8 min_decksize,
                                 gint8 max_influence) {
  g_assert(card->type == RUNNER_ID || card->type == CORP_ID);
  g_assert(!card_is_neutral(card));
  g_assert(min_decksize >= 0);
  g_assert(max_influence >= 0);

  card->min_decksize = min_decksize;
  card->max_influence = max_influence;
  return card;
}

struct card* card_fill_runner_id(struct card *card,
                                 gint8 min_decksize,
                                 gint8 max_influence,
                                 gint8 base_link) {
  g_assert(card_is_runner(card));
  g_assert(card->type == RUNNER_ID);
  g_assert(base_link >= 0);

  card->base_link = base_link;
  return card_fill_id(card, min_decksize, max_influence);
}

/* Either "X" or totally composed of digits. */
static gboolean is_valid_cost(const gchar *cost) {
  if (strcmp(cost, "X") == 0) return TRUE;
  for (int i = 0; cost[i] != '\0'; i++) {
    if (!g_ascii_isdigit(cost[i])) return FALSE;
  }
  return TRUE;
}

struct card* card_fill_costed(struct card *card,
                              const gchar *cost,
                              gint8 influence_cost) {
  g_assert((card_is_runner(card) && (card->type == RUNNER_EVENT
                                     || card->type == RUNNER_HARDWARE
                                     || card->type == RUNNER_PROGRAM
                                     || card->type == RUNNER_ICEBREAKER
                                     || card->type == RUNNER_RESOURCE))
           || (card_is_corp(card) && (card->type == CORP_ASSET
                                      || card->type == CORP_UPGRADE
                                      || card->type == CORP_OPERATION
                                      || card->type == CORP_ICE)));
  g_assert(cost != NULL && is_valid_cost(cost));
  if (card_is_neutral(card)) {
    g_assert(influence_cost == 0);
  } else {
    g_assert(influence_cost > 0);
  }

  if (card->cost != NULL) g_free(card->cost);
  card->cost = g_strdup(cost);
  card->influence_cost = influence_cost;
  return card;
}

struct card* card_fill_program(struct card *card,
                               const gchar *cost,
                               gint8 influence_cost,
                               gint8 memory_cost) {
  g_assert(card_is_runner(card));
  g_assert(card->type == RUNNER_PROGRAM
           || card->type == RUNNER_ICEBREAKER);
  g_assert(memory_cost > 0);

  card->memory_cost = memory_cost;
  return card_fill_costed(card, cost, influence_cost);
}

struct card* card_fill_icebreaker(struct card *card,
                                  const gchar *cost,
                                  gint8 influence_cost,
                                  gint8 memory_cost,
                                  gint8 strength) {
  g_assert(card->type == RUNNER_ICEBREAKER);
  g_assert(strength >= 0);

  card->strength = strength;
  return card_fill_program(card, cost, influence_cost, memory_cost);
}

struct card* card_fill_corp_id(struct card *card,
                               gint8 min_decksize,
                               gint8 max_influence) {
  g_assert(card_is_corp(card));
  g_assert(card->type == CORP_ID);

  return card_fill_id(card, min_decksize, max_influence);
}

struct card* card_fill_agenda(struct card *card,
                              const gchar *cost,
                              gint8 agenda_points) {
  g_assert(card_is_corp(card));
  g_assert(card->type == CORP_AGENDA);
  errno = 0;
  char* end;
  gint64 i_cost = g_ascii_strtoll(cost, &end, 10);
  g_assert(errno == 0 && i_cost > 0 && *end == '\0');
  g_assert(agenda_points > 0);

  if (card->cost != NULL) g_free(card->cost);
  card->cost = g_strdup(cost);
  card->agenda_points = agenda_points;
  return card;
}

struct card* card_fill_asset_upgrade(struct card *card,
                                     const gchar *cost,
                                     gint8 influence_cost,
                                     gint8 trash_cost) {
  g_assert(card_is_corp(card));
  g_assert(card->type == CORP_ASSET
           || card->type == CORP_UPGRADE);

  card->trash_cost = trash_cost;
  return card_fill_costed(card, cost, influence_cost);
}

struct card* card_fill_ice(struct card *card,
                           const gchar *cost,
                           gint8 influence_cost,
                           gint8 strength) {
  g_assert(card_is_corp(card));
  g_assert(card->type == CORP_ICE);

  card->strength = strength;
  return card_fill_costed(card, cost, influence_cost);
}

void card_free(struct card *card) {
  if (card == NULL) return;
  g_free(card->type_str);
  g_free(card->name);
  g_free(card->text);
  g_free(card->flavor);
  g_free(card->illustrator);
  g_free(card->cost);
  g_slice_free(struct card, card);
}

gboolean card_is_runner(const struct card *card) {
  return (card->faction == RUNNER_ANARCH
          || card->faction == RUNNER_CRIMINAL
          || card->faction == RUNNER_SHAPER
          || card->faction == RUNNER_NEUTRAL);
}

gboolean card_is_corp(const struct card *card) {
  return (card->faction == CORP_HAAS_BIOROID
          || card->faction == CORP_JINTEKI
          || card->faction == CORP_NBN
          || card->faction == CORP_WEYLAND
          || card->faction == CORP_NEUTRAL);
}

gboolean card_is_neutral(const struct card *card) {
  return (card->faction == RUNNER_NEUTRAL
          || card->faction == CORP_NEUTRAL);
}

gboolean card_has_agenda_points(const struct card *card) {
  return card->type == CORP_AGENDA;
}

gboolean card_has_base_link(const struct card *card) {
  return card->type == RUNNER_ID;
}

gboolean card_has_cost(const struct card *card) {
  return (card->type != RUNNER_ID
          && card->type != CORP_ID);
}

gboolean card_has_influence_cost(const struct card *card) {
  return (card->type != RUNNER_ID
          && card->type != CORP_ID
          && card->type != CORP_AGENDA
          && card->faction != RUNNER_NEUTRAL
          && card->faction != CORP_NEUTRAL);
}

gboolean card_has_max_influence(const struct card *card) {
  return (card->type == RUNNER_ID
          || card->type == CORP_ID);
}

gboolean card_has_memory_cost(const struct card *card) {
  return (card->type == RUNNER_PROGRAM
          || card->type == RUNNER_ICEBREAKER);
}

gboolean card_has_min_decksize(const struct card *card) {
  return (card->type == RUNNER_ID
          || card->type == CORP_ID);
}

gboolean card_has_strength(const struct card *card) {
  return (card->type == RUNNER_ICEBREAKER
          || card->type == CORP_ICE);
}

gboolean card_has_trash_cost(const struct card *card) {
  return (card->type == CORP_ASSET
          || card->type == CORP_UPGRADE);
}

gchar* card_render_name(const struct card *card) {
  return g_strdup_printf("%s%s", card->unique ? "\u25c6 " : "", card->name);
}
