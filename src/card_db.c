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

#include "card_db.h"

#include <jansson.h>

#include "card.h"
#include "hash.h"

struct card_db* card_db_new(void) {
  struct card_db *db = g_new(struct card_db, 1);
  db->cards = NULL;
  return db;
}

void card_db_free(struct card_db *db) {
  if (db == NULL) return;
  void free_card(gpointer p) { card_free(p); }
  g_list_free_full(db->cards, free_card);
  g_free(db);
}

static struct card* fill_card(struct card *card, json_t *j_card) {
  gint8 agenda_points, base_link, cost, influence_cost, max_influence,
    memory_cost, min_decksize, strength, trash_cost;
  const struct {
    const char *field;
    gint8 *p_int;
    gboolean (*predicate)(const struct card*);
  } *p, field_table[] = {
    { "agenda_points" , &agenda_points , card_has_agenda_points  },
    { "base_link"     , &base_link     , card_has_base_link      },
    { "cost"          , &cost          , card_has_cost           },
    { "influence_cost", &influence_cost, card_has_influence_cost },
    { "max_influence" , &max_influence , card_has_max_influence  },
    { "memory_cost"   , &memory_cost   , card_has_memory_cost    },
    { "min_decksize"  , &min_decksize  , card_has_min_decksize   },
    { "strength"      , &strength      , card_has_strength       },
    { "trash_cost"    , &trash_cost    , card_has_trash_cost     },
    { NULL            , NULL           , NULL                    }
  };

  // TODO: something better than g_assert().
  for (p = field_table; p->field != NULL; p++) {
    if (!p->predicate(card)) continue;
    json_t *json_obj = json_object_get(j_card, p->field);
    g_assert(json_is_integer(json_obj));
    *p->p_int = json_integer_value(json_obj);
  }

  switch (card->type) {
  case RUNNER_ID:
    return card_fill_runner_id
      (card, min_decksize, max_influence, base_link);
  case RUNNER_EVENT:
  case RUNNER_HARDWARE:
  case RUNNER_RESOURCE:
  case CORP_OPERATION:
    return card_fill_costed
      (card, cost, influence_cost);
  case RUNNER_PROGRAM:
    return card_fill_program
      (card, cost, influence_cost, memory_cost);
  case RUNNER_ICEBREAKER:
    return card_fill_icebreaker
      (card, cost, influence_cost, memory_cost, strength);
  case CORP_ID:
    return card_fill_corp_id
      (card, min_decksize, max_influence);
  case CORP_AGENDA:
    return card_fill_agenda
      (card, cost, agenda_points);
  case CORP_ASSET:
  case CORP_UPGRADE:
    return card_fill_asset_upgrade
      (card, cost, influence_cost, trash_cost);
  case CORP_ICE:
    return card_fill_ice
      (card, cost, influence_cost, strength);
  }
  g_assert_not_reached();
}

static struct card* load_card(json_t *j_card, const char *set_name) {
  json_t *j_faction = json_object_get(j_card, "faction");
  // TODO: something better than g_assert().
  g_assert(json_is_string(j_faction));
  const char *str_faction = json_string_value(j_faction);
  enum faction faction;
  gboolean ok = hash_faction_name(str_faction, &faction);
  g_assert(ok);

  json_t *j_type = json_object_get(j_card, "type");
  g_assert(json_is_string(j_type));

  json_t *j_number = json_object_get(j_card, "number");
  g_assert(json_is_integer(j_number));

  json_t *j_quantity = json_object_get(j_card, "quantity");
  gint8 quantity = 3;
  if (j_quantity != NULL) {
    g_assert(json_is_integer(j_quantity));
    quantity = json_integer_value(j_quantity);
  }

  json_t *j_unique = json_object_get(j_card, "unique");
  gboolean unique = FALSE;
  if (j_unique != NULL) {
    g_assert(json_is_boolean(j_unique));
    unique = json_is_true(j_unique);
  }

  json_t *j_name = json_object_get(j_card, "name");
  g_assert(json_is_string(j_name));

  json_t *j_text = json_object_get(j_card, "text");
  g_assert(json_is_string(j_text));

  json_t *j_flavor = json_object_get(j_card, "flavor");
  const char *flavor = NULL;
  if (j_flavor != NULL) {
    g_assert(json_is_string(j_flavor));
    flavor = json_string_value(j_flavor);
  }

  json_t *j_illustrator = json_object_get(j_card, "illustrator");
  const char* illustrator = NULL;
  if (j_illustrator != NULL) {
    g_assert(json_is_string(j_illustrator));
    illustrator = json_string_value(j_illustrator);
  }

  struct card *card = card_new(faction,
                               json_string_value(j_type),
                               set_name,
                               json_integer_value(j_number),
                               quantity,
                               unique,
                               json_string_value(j_name),
                               json_string_value(j_text),
                               flavor,
                               illustrator);

  return fill_card(card, j_card);
}

void card_db_load_file(struct card_db *db, const gchar *path) {
  json_error_t err;
  json_t *j_set = json_load_file(path, 0, &err);
  // TODO: Log properly.
  if(!json_is_object(j_set)) {
    printf("%s:%d:%d: %s\n", err.source, err.line, err.column, err.text);
  }
  // TODO: something better than g_assert().
  g_assert(json_is_object(j_set));

  json_t *j_set_name = json_object_get(j_set, "name");
  g_assert(json_is_string(j_set_name));
  const char *set_name = json_string_value(j_set_name);
  g_message("Loading set: %s", set_name);

  json_t *j_cards = json_object_get(j_set, "cards");
  g_assert(json_is_array(j_cards));
  gsize n = json_array_size(j_cards);

  GList *cards = NULL;
  for (uint i = 0; i < n; i++) {
    json_t *j_card = json_array_get(j_cards, i);
    g_assert(json_is_object(j_card));
    // TODO, XXX: this will leave card pointing to unitialised memory!
    // TODO, XXX: Fix it ASAP by adding card sets to card_db!
    struct card *card = load_card(j_card, set_name);
    g_debug("Loaded card: %s", card->name);
    cards = g_list_prepend(cards, card);
  }

  json_decref(j_set);

  cards = g_list_reverse(cards);
  db->cards = g_list_concat(db->cards, cards);
}
