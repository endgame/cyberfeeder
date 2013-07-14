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
#include "widget_helpers.h"

GtkWidget* widget_wrap_frame(GtkWidget *widget) {
  g_assert(widget != NULL);
  GtkWidget *rv = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(rv), GTK_SHADOW_IN);
  gtk_container_add(GTK_CONTAINER(rv), widget);
  return rv;
}

GtkWidget* widget_wrap_scroller(GtkWidget *widget) {
  g_assert(widget != NULL);
  GtkWidget *rv = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(rv), widget);
  return rv;
}
