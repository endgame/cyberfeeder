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

static struct card* card_fill_id(struct card *card,
                                 gint8 min_decksize,
                                 gint8 max_influence) {
  if (card->type != RUNNER_ID && card->type != CORP_ID) {
    load_error_card(card, "Must be an \"Identity\" card");
    goto err;
  }
  if (card_is_neutral(card)) {
    load_error_card(card, "Identities cannot be neutral");
    goto err;
  }
  if (min_decksize < 0) {
    load_error_card(card, "Minimum deck size must be positive, got %d",
                    min_decksize);
    goto err;
  }
  if (max_influence < 0) {
    load_error_card(card, "Maximum influence must be positive, got %d",
                    max_influence);
    goto err;
  }

  card->min_decksize = min_decksize;
  card->max_influence = max_influence;
  return card;
 err:
  card_free(card);
  return NULL;
}

struct card* card_fill_runner_id(struct card *card,
                                 gint8 min_decksize,
                                 gint8 max_influence,
                                 gint8 base_link) {
  if (!card_is_runner(card)) {
    load_error_card(card, "Must belong to a Runner faction");
    goto err;
  }
  if (card->type != RUNNER_ID) {
    load_error_card(card, "Must be a Runner Identity card");
    goto err;
  }
  if (base_link < 0) {
    load_error_card(card, "Base link must be positive, got %d", base_link);
    goto err;
  }

  card->base_link = base_link;
  return card_fill_id(card, min_decksize, max_influence);
 err:
  card_free(card);
  return NULL;
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
  if (card_is_runner(card)) {
    if (card->type != RUNNER_EVENT
        && card->type != RUNNER_HARDWARE
        && card->type != RUNNER_PROGRAM
        && card->type != RUNNER_ICEBREAKER
        && card->type != RUNNER_RESOURCE) {
      load_error_card(card,
                      "Costed Runner cards must be events, hardware,"
                      " programs, icebreakers or resources");
      goto err;
    }
  } else {
    if (card->type != CORP_ASSET
        && card->type != CORP_UPGRADE
        && card->type != CORP_OPERATION
        && card->type != CORP_ICE) {
      load_error_card(card,
                      "Costed Corp cards must be assets, upgrades,"
                      " operations or ICE.");
      goto err;
    }
  }
  if (cost == NULL || !is_valid_cost(cost)) {
    load_error_card
      (card, "Invalid cost, must be a positive integer or \"X\", got %s", cost);
    goto err;
  }
  if (card_is_neutral(card)) {
    if (influence_cost != 0) {
      load_error_card
        (card, "Influence cost for neutral cards must be 0, got %d",
         influence_cost);
      goto err;
    }
  } else {
    if (influence_cost < 0) {
      load_error_card
        (card, "Influence cost for non-neutral cards must be positive, got %d",
         influence_cost);
      goto err;
    }
  }

  if (card->cost != NULL) g_free(card->cost);
  card->cost = g_strdup(cost);
  card->influence_cost = influence_cost;
  return card;

 err:
  card_free(card);
  return NULL;
}

struct card* card_fill_program(struct card *card,
                               const gchar *cost,
                               gint8 influence_cost,
                               gint8 memory_cost) {
  if (!card_is_runner(card)) {
    load_error_card(card, "Only Runner factions can have programs");
    goto err;
  }
  if (card->type != RUNNER_PROGRAM && card->type != RUNNER_ICEBREAKER) {
    load_error_card(card, "Must have type \"Program\"");
    goto err;
  }
  if (memory_cost < 1) {
    load_error_card(card,
                    "Memory cost must be at least 1, got %d", memory_cost);
    goto err;
  };

  card->memory_cost = memory_cost;
  return card_fill_costed(card, cost, influence_cost);
 err:
  card_free(card);
  return NULL;
}

struct card* card_fill_icebreaker(struct card *card,
                                  const gchar *cost,
                                  gint8 influence_cost,
                                  gint8 memory_cost,
                                  gint8 strength) {
  if (card->type != RUNNER_ICEBREAKER) {
    load_error_card(card, "Must be a \"Program: Icebreaker\" card");
    goto err;
  }
  if (strength < 0) {
    load_error_card(card, "Strength must be non-negative, got %d", strength);
    goto err;
  }

  card->strength = strength;
  return card_fill_program(card, cost, influence_cost, memory_cost);
 err:
  card_free(card);
  return NULL;
}

struct card* card_fill_corp_id(struct card *card,
                               gint8 min_decksize,
                               gint8 max_influence) {
  if (!card_is_corp(card)) {
    load_error_card(card, "Must belong to a Corp faction");
    goto err;
  }
  if (card->type != CORP_ID) {
    load_error_card(card, "Must be a Corp Identity card");
    goto err;
  }

  return card_fill_id(card, min_decksize, max_influence);
 err:
  card_free(card);
  return NULL;
}

struct card* card_fill_agenda(struct card *card,
                              const gchar *cost,
                              gint8 agenda_points) {
  if (!card_is_corp(card)) {
    load_error_card(card, "Must belong to a corp faction");
    goto err;
  }
  if (card->type != CORP_AGENDA) {
    load_error_card(card, "Must have type \"Agenda\"");
    goto err;
  }
  errno = 0;
  char* end;
  gint64 i_cost = g_ascii_strtoll(cost, &end, 10);
  if (errno != 0 || i_cost <= 0 || *end != '\0') {
    load_error_card(card,
                    "Agenda costs must be a positive number, got %s", cost);
    goto err;
  }
  if (agenda_points <= 0) {
    load_error_card(card,
                    "Agendas must be worth at least 1 agenda point, got %d",
                    agenda_points);
  }

  if (card->cost != NULL) g_free(card->cost);
  card->cost = g_strdup(cost);
  card->agenda_points = agenda_points;
  return card;
 err:
  card_free(card);
  return NULL;
}

struct card* card_fill_asset_upgrade(struct card *card,
                                     const gchar *cost,
                                     gint8 influence_cost,
                                     gint8 trash_cost) {
  if (!card_is_corp(card)) {
    load_error_card(card, "Only Corp factions can have assets/upgrades");
    goto err;
  }
  if (card->type != CORP_ASSET && card->type != CORP_UPGRADE) {
    load_error_card(card, "Must have type \"Asset\" or \"Upgrade\"");
    goto err;
  }
  if (trash_cost < 0) {
    load_error_card(card,
                    "Trash cost must be non-negative, got %d", trash_cost);
    goto err;
  }

  card->trash_cost = trash_cost;
  return card_fill_costed(card, cost, influence_cost);
 err:
  card_free(card);
  return NULL;
}

struct card* card_fill_ice(struct card *card,
                           const gchar *cost,
                           gint8 influence_cost,
                           gint8 strength) {
  if (!card_is_corp(card)) {
    load_error_card(card, "Only Corp factions can have ice");
    goto err;
  }
  if (card->type != CORP_ICE) {
    load_error_card(card, "Must have type \"ICE\"");
    goto err;
  }

  card->strength = strength;
  return card_fill_costed(card, cost, influence_cost);
 err:
  card_free(card);
  return NULL;
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
