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

/* A single set of cards - the core set, or a single data pack. */

#ifndef CARD_SET_H
#define CARD_SET_H

#include <glib.h>
#include <string.h>

struct card_set {
  gchar* name;
  GPtrArray* /* of struct card*, owned */ cards;
};

struct card_set* card_set_new(const gchar* name);
void card_set_free(struct card_set *set);

struct card_set* card_set_load_file(const gchar *path);

#endif
