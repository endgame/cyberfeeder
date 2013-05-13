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
#include "pages.h"

#include "text_view_helpers.h"

GtkWidget* tmi(void) {
  GtkTextView *text_view =
    text_view_make_uneditable(GTK_TEXT_VIEW(gtk_text_view_new()));
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
  GtkTextIter iter;
  gtk_text_buffer_get_start_iter(buffer, &iter);
  text_buffer_insert_markup
    (buffer, &iter,
     "<big>Cyberfeeder - a utility for Android:Netrunner</big>\n"
     "Copyright (C) 2013  Jack Kelly\n"
     "This program comes with <b>ABSOLUTELY NO WARRANTY</b>.\n"
     "This is free software, and you are welcome to redistribute it "
     "under certain conditions; see the GNU GPL (version 3 or later) "
     "for details.");

  GtkWidget *scroller = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scroller), GTK_WIDGET(text_view));

  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  gtk_container_add(GTK_CONTAINER(frame), scroller);
  return frame;
}
