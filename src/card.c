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
  g_assert(illustrator != NULL);

  struct card *card = g_new(struct card, 1);
  card->faction = faction;

  if (g_str_has_prefix(type, "Identity"))
    card->type = (card_is_runner(card) ? RUNNER_ID : CORP_ID);
  else if (g_str_has_prefix(type, "Event"))
    card->type = RUNNER_EVENT;
  else if (g_str_has_prefix(type, "Hardware"))
    card->type = RUNNER_HARDWARE;
  else if (g_str_has_prefix(type, "Program: Icebreaker"))
    card->type = RUNNER_PROGRAM;
  else if (g_str_has_prefix(type, "Program"))
    card->type = RUNNER_ICEBREAKER;
  else if (g_str_has_prefix(type, "Resource"))
    card->type = RUNNER_RESOURCE;
  else if (g_str_has_prefix(type, "Agenda"))
    card->type = CORP_AGENDA;
  else if (g_str_has_prefix(type, "Asset"))
    card->type = CORP_ASSET;
  else if (g_str_has_prefix(type, "Upgrade"))
    card->type = CORP_UPGRADE;
  else if (g_str_has_prefix(type, "Operation"))
    card->type = CORP_OPERATION;
  else if (g_str_has_prefix(type, "ICE"))
    card->type = CORP_ICE;
  else
    g_error("Unknown card type.");

  card->type_str = g_strdup(type);
  card->set = set;
  card->number = number;
  card->quantity = quantity;
  card->unique = unique;
  card->name = g_strdup(name);
  card->text = g_strdup(text);
  card->flavor = g_strdup(flavor);
  card->illustrator = g_strdup(illustrator);
  return card;
}

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

struct card* card_fill_runnner_id(struct card *card,
                                  gint8 min_decksize,
                                  gint8 max_influence,
                                  gint8 base_link) {
  g_assert(card_is_runner(card));
  g_assert(card->type == RUNNER_ID);
  g_assert(base_link >= 0);

  card->base_link = base_link;
  return card_fill_id(card, min_decksize, max_influence);
}

struct card* card_fill_costed(struct card *card,
                              gint8 cost,
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
  g_assert(cost >= 0);
  g_assert(influence_cost == 0 || !card_is_neutral(card));

  card->cost = cost;
  card->influence_cost = cost;
  return card;
}

struct card* card_fill_program(struct card *card,
                               gint8 cost,
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
                                  gint8 cost,
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
                              gint8 cost,
                              gint8 agenda_points) {
  g_assert(card_is_corp(card));
  g_assert(card->type == CORP_AGENDA);
  g_assert(cost > 0);
  g_assert(agenda_points > 0);

  card->cost = cost;
  card->agenda_points = agenda_points;
  return card;
}

struct card* card_fill_asset_upgrade(struct card *card,
                                     gint8 cost,
                                     gint8 influence_cost,
                                     gint8 trash_cost) {
  g_assert(card_is_corp(card));
  g_assert(card->type == CORP_ASSET
           || card->type == CORP_UPGRADE);

  card->trash_cost = trash_cost;
  return card_fill_costed(card, cost, influence_cost);
}

struct card* card_fill_ice(struct card *card,
                           gint8 cost,
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
  g_free(card);
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
