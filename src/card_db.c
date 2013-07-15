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

#include "card.h"
#include "card_set.h"

static void free_set(gpointer p) { card_set_free(p); }

struct card_db* card_db_new(void) {
  struct card_db *db = g_slice_new(struct card_db);
  db->sets = g_ptr_array_new_with_free_func(free_set);
  db->all_cards = g_hash_table_new(g_str_hash, g_str_equal);
  return db;
}

void card_db_free(struct card_db *db) {
  if (db == NULL) return;
  g_ptr_array_free(db->sets, TRUE);
  g_hash_table_unref(db->all_cards);
  g_slice_free(struct card_db, db);
}

void card_db_add_set(struct card_db *db, struct card_set *set) {
  g_assert(db != NULL);
  g_assert(set != NULL);
  g_ptr_array_add(db->sets, set);

  for (guint i = 0; i < set->cards->len; i++) {
    struct card *card = g_ptr_array_index(set->cards, i);
    g_hash_table_insert(db->all_cards, card->name, card);
  }
}
