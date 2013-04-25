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

/* Card info is mainly displayed via a GtkTextView. These functions
   help with that. */

#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#include <gtk/gtk.h>

struct card;

void ui_helpers_text_buffer_add_card(GtkTextBuffer *text,
                                     GtkTextIter *iter,
                                     const struct card *card);

#endif
