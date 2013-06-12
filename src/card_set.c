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
#include "card_set.h"

#include <glib/gprintf.h>
#include <jansson.h>
#include <pango/pango.h>

#include "card.h"
#include "hash.h"
#include "json.h"
#include "load_error.h"

static void free_card(gpointer p) { card_free(p); }

struct card_set* card_set_new(const gchar* name) {
  g_assert(name != NULL);
  struct card_set *set = g_slice_new(struct card_set);
  set->name = g_strdup(name);
  set->cards = g_ptr_array_new_with_free_func(free_card);
  return set;
}

void card_set_free(struct card_set *set) {
  if (set == NULL) return;
  g_free(set->name);
  g_ptr_array_free(set->cards, TRUE);
  g_slice_free(struct card_set, set);
}

static struct card* fill_card(struct card *card, json_t *j_card) {
  gint8 agenda_points, base_link, influence_cost = 0, max_influence,
    memory_cost, min_decksize, strength, trash_cost;
  const struct {
    const char *field;
    gint8 *p_int;
    gboolean (*predicate)(const struct card*);
  } *p, field_table[] = {
    { "agenda_points" , &agenda_points , card_has_agenda_points  },
    { "base_link"     , &base_link     , card_has_base_link      },
    { "influence_cost", &influence_cost, card_has_influence_cost },
    { "max_influence" , &max_influence , card_has_max_influence  },
    { "memory_cost"   , &memory_cost   , card_has_memory_cost    },
    { "min_decksize"  , &min_decksize  , card_has_min_decksize   },
    { "strength"      , &strength      , card_has_strength       },
    { "trash_cost"    , &trash_cost    , card_has_trash_cost     },
    { NULL            , NULL           , NULL                    }
  };

  for (p = field_table; p->field != NULL; p++) {
    if (!p->predicate(card)) continue;
    json_t *json_obj = json_object_get_checked(j_card, p->field, JSON_INTEGER);
    if (json_obj == NULL) goto err;
    *p->p_int = json_integer_value(json_obj);
  }

  /* Handle cost separately; "X" is a valid cost (Psychographics). */
  gboolean cost_is_x = FALSE;
  int cost = -1;
  if (card_has_cost(card)) {
    if (!json_object_get_int_or_x(j_card, "cost", &cost, &cost_is_x)) goto err;
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
      (card, cost_is_x, cost, influence_cost);
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
  err:
  card_free(card);
  return NULL;
}

static struct card* load_card(json_t *j_card, const char *set_name) {
  g_assert(set_name != NULL);
  json_t *j_faction = json_object_get_checked(j_card, "faction", JSON_STRING);
  if (j_faction == NULL) goto err_0;

  const char *str_faction = json_string_value(j_faction);
  enum faction faction;
  gboolean ok = hash_faction_name(str_faction, &faction);
  if (!ok) {
    load_error(".faction: Invalid faction \"%s\"", str_faction);
    goto err_0;
  }

  json_t *j_type = json_object_get_checked(j_card, "type", JSON_STRING);
  if (j_type == NULL) goto err_0;

  json_t *j_number = json_object_get_checked(j_card, "number", JSON_INTEGER);
  if (j_number == NULL) goto err_0;
  json_int_t number = json_integer_value(j_number);
  if (number < 0) {
    load_error(".number: Should be positive, got %" JSON_INTEGER_FORMAT,
               number);
    goto err_0;
  }
  if (number > 255) {
    load_error(".number: Should be <= 255, got %" JSON_INTEGER_FORMAT, number);
    goto err_0;
  }

  gint8 quantity = json_object_get_int_default(j_card, "quantity", 3);
  if (quantity < 0) {
    load_error(".quantity: Should be positive, got %d", quantity);
    goto err_0;
  }
  if (quantity > 3) {
    load_error(".quantity: Should be <= 3, got %d", quantity);
    goto err_0;
  }

  gboolean unique = json_object_get_bool_default(j_card, "unique", FALSE);

  json_t *j_name = json_object_get_checked(j_card, "name", JSON_STRING);
  if (j_name == NULL) goto err_0;

  GError *err = NULL;

  json_t *j_text = json_object_get_checked(j_card, "text", JSON_STRING);
  if (j_text == NULL) goto err_0;
  const char *text = json_string_value(j_text);
  if (!pango_parse_markup(text, -1, 0, NULL, NULL, NULL, &err)) {
    load_error(".text: %s", err->message);
    g_error_free(err);
    goto err_0;
  }

  gchar *flavor = json_object_get_string_maybe(j_card, "flavor");
  if (flavor != NULL
      && !pango_parse_markup(flavor, -1, 0, NULL, NULL, NULL, &err)) {
    load_error(".flavor: %s", err->message);
    g_error_free(err);
    goto err_1;
  }

  gchar *illustrator = json_object_get_string_maybe(j_card, "illustrator");

  struct card *card = card_new(faction,
                               json_string_value(j_type),
                               set_name,
                               number,
                               quantity,
                               unique,
                               json_string_value(j_name),
                               text,
                               flavor,
                               illustrator);
  if (card == NULL) goto err_2;
  g_free(flavor);
  g_free(illustrator);
  return fill_card(card, j_card);

 err_2:
  g_free(illustrator);
 err_1:
  g_free(flavor);
 err_0:
  return NULL;
}

struct card_set* card_set_load_file(const gchar *path) {
  json_error_t err;
  json_t *j_set = json_load_file(path, 0, &err);

  if (j_set == NULL) {
    load_error("%s (%d:%d): %s",
               path, err.line, err.column, err.text);
    goto err_0;
  } else if (!json_is_object(j_set)) {
    load_error("%s: Expected an object, got %s",
               path, json_typename(json_typeof(j_set)));
    goto err_1;
  }

  json_t *j_set_name = json_object_get_checked(j_set, "name", JSON_STRING);
  if (j_set_name == NULL) goto err_1;
  const char *set_name = json_string_value(j_set_name);

  json_t *j_cards = json_object_get_checked(j_set, "cards", JSON_ARRAY);
  if (j_cards == NULL) goto err_1;
  gsize n = json_array_size(j_cards);

  struct card_set* set = card_set_new(set_name);
  g_message("Loading set: %s", set_name);
  load_error_push_source(".cards");
  for (uint i = 0; i < n; i++) {
    json_t *j_card = json_array_get_checked(j_cards, i, JSON_OBJECT);
    if (j_card == NULL) continue;

    load_error_push_source("[%d]", i);
    struct card * card = load_card(j_card, set->name);
    if (card != NULL) {
      g_debug("Loaded card: %s", card->name);
      g_ptr_array_add(set->cards, card);
    }
    load_error_pop_source();
  }
  load_error_pop_source();

  json_decref(j_set);
  return set;

 err_1:
  json_decref(j_set);
 err_0:
  return NULL;
}
