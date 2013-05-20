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
#include "load_error.h"

static GPtrArray /* of gchar*, owned */ *errors = NULL;

static inline void setup(void) {
  if (errors == NULL) errors = g_ptr_array_new_with_free_func(g_free);
}

void load_error(const char *format, ...) {
  setup();
  va_list args;
  va_start(args, format);
  g_ptr_array_add(errors, g_strdup_vprintf(format, args));
  va_end(args);
}

void load_error_card(const struct card *card,
                     const char *format, ...);

/* Show all errors in a dialog, then free allocated memory. */
void load_error_show(void) {
  if (errors == NULL) return;

  /* TODO: As a dialog */
  for (guint i = 0; i < errors->len; i++) {
    puts(g_ptr_array_index(errors, i));
  }

  g_ptr_array_free(errors, TRUE);
  errors = NULL;
}
