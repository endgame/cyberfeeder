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

/* The complete database of cards - the core and every pack. */

#ifndef CARD_DB_H
#define CARD_DB_H

#include <gtk/gtk.h>

struct card_set;

struct card_db {
  GPtrArray* /* of struct card_set*, owned */ sets;
};

struct card_db* card_db_new(void);
void card_db_free(struct card_db *db);

/* Takes ownership of set. */
void card_db_add_set(struct card_db *db, struct card_set *set);
#endif
