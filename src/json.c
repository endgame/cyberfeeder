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
#include "json.h"

#include <glib.h>

#include "load_error.h"

json_t* json_array_get_checked(json_t *arr,
                               size_t i,
                               json_type type) {
  g_assert(json_is_array(arr));
  json_t *rv = json_array_get(arr, i);
  if (rv == NULL) {
    load_error("[%d]: Array element missing", i);
    return NULL;
  }
  if (json_typeof(rv) != type) {
    load_error("[%d]: Expected type %s, got %s",
               i, json_typename(type), json_typename(json_typeof(rv)));
    return NULL;
  }
  return rv;
}

json_t* json_object_get_checked(json_t *obj,
                                const char *field,
                                json_type type) {
  g_assert(json_is_object(obj));
  json_t *rv = json_object_get(obj, field);
  if (rv == NULL) {
    load_error(".%s: Field missing", field);
    return NULL;
  }
  if (json_typeof(rv) != type) {
    load_error(".%s: Expected type %s, got %s",
               field, json_typename(type), json_typename(json_typeof(rv)));
    return NULL;
  }
  return rv;
}

gboolean json_object_get_bool_default(json_t *obj,
                                      const char *field,
                                      gboolean def) {
  g_assert(json_is_object(obj));
  json_t *val = json_object_get(obj, field);
  if (val == NULL) return def;
  if (!json_is_boolean(val)) {
    load_error(".%s: Expected type bool, got %s",
               field, json_typename(json_typeof(val)));
    return def;
  }
  return json_is_true(val);
}

int json_object_get_int_default(json_t *obj,
                                const char *field,
                                int def) {
  g_assert(json_is_object(obj));
  json_t *val = json_object_get(obj, field);
  if (val == NULL) return def;
  if (!json_is_integer(val)) {
    load_error(".%s: Expected type int, got %s",
               field, json_typename(json_typeof(val)));
    return def;
  }
  return json_integer_value(val);
}

gchar* json_object_get_string_maybe(json_t *obj,
                                    const char *field) {
  g_assert(json_is_object(obj));
  json_t *val = json_object_get(obj, field);
  if (val == NULL) return NULL;
  if (!json_is_string(val)) {
    load_error(".%s: Expected type string, got %s",
               field, json_typename(json_typeof(val)));
    return NULL;
  }
  return g_strdup(json_string_value(val));
}

const char* json_typename(json_type type) {
  switch (type) {
  case JSON_OBJECT:  return "object";
  case JSON_ARRAY:   return "array";
  case JSON_STRING:  return "string";
  case JSON_INTEGER: return "integer";
  case JSON_REAL:    return "real";
  case JSON_TRUE:
  case JSON_FALSE:   return "boolean";
  case JSON_NULL:    return "null";
  default: g_assert_not_reached();
  }
}
