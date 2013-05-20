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

/* Track loading errors. */

#ifndef LOAD_ERROR_H
#define LOAD_ERROR_H

#include <glib.h>

struct card;

void load_error(const char *format, ...) G_GNUC_PRINTF(1, 2);
void load_error_card(const struct card *card,
                     const char *format, ...) G_GNUC_PRINTF(2, 3);
/* Show all errors in a dialog, then free allocated memory. */
void load_error_show(void);

#endif
