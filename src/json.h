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

/* Extra json functions. */

#ifndef JSON_H
#define JSON_H

#include <glib.h>
#include <jansson.h>

/* Logs with load_error and returns NULL if things are wrong. */
json_t* json_array_get_checked(json_t *arr,
                               size_t i,
                               json_type type);

/* Logs with load_error and returns NULL if things go wrong. */
json_t* json_object_get_checked(json_t *obj,
                                const char *field,
                                json_type type);

/* Object accessors with defaults. */
gboolean json_object_get_bool_default(json_t *obj,
                                      const char *field,
                                      gboolean def);

int json_object_get_int_default(json_t *obj,
                                const char *field,
                                int def);

/* Returns NULL if not found. Free with g_free(). */
gchar* json_object_get_string_maybe(json_t *obj,
                                    const char *field);

/* For e.g. "cost" field of cards, where either an integer or the
   string "X" are both valid. Sets both *i and *is_x, unless there's
   an error (e.g., missing field), in which case it returns FALSE. */
gboolean json_object_get_int_or_x(json_t *obj,
                                  const char *field,
                                  int *i,
                                  gboolean *is_x);

const char* json_typename(json_type type);

#endif
