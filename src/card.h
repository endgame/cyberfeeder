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

#ifndef CARD_H
#define CARD_H

#include <glib.h>

enum faction {
  RUNNER_ANARCH,
  RUNNER_CRIMINAL,
  RUNNER_SHAPER,
  RUNNER_NEUTRAL,

  CORP_HAAS_BIOROID,
  CORP_JINTEKI,
  CORP_NBN,
  CORP_WEYLAND,
  CORP_NEUTRAL,
};

const gchar* faction_to_string(enum faction faction);

enum card_type {
  RUNNER_ID,
  RUNNER_EVENT,
  RUNNER_HARDWARE,
  RUNNER_PROGRAM,
  RUNNER_ICEBREAKER,
  RUNNER_RESOURCE,

  CORP_ID,
  CORP_AGENDA,
  CORP_ASSET,
  CORP_UPGRADE,
  CORP_OPERATION,
  CORP_ICE
};

struct card {
  gchar *name;
  const gchar *set; /* Not owned. Held by struct card_db */
  gint8 number; /* Noise = 1, Deja Vu = 2, ... */
  gint8 quantity; /* == 3, except for certain Core cards */

  enum faction faction;
  enum card_type type;
  gchar *type_str;

  gboolean unique; /* e.g., Ice Carver */
  gchar *text;
  gchar *flavor; /* May be NULL */
  gchar *illustrator; /* May be NULL (Corp IDs) */

  gboolean cost_is_x; /* For e.g., Psychographics */
  gint8 cost; /* Rez cost for assets/upgrades/ice; advance cost for
                 agendas. */
  gint8 agenda_points; /* Agendas only */
  gint8 influence_cost; /* == 0 for agendas/neutral */
  gint8 trash_cost; /* Assets/upgrades only */
  gint8 memory_cost; /* Programs only */

  gboolean strength_is_x; /* For e.g., Darwin */
  gint8 strength; /* Ice/icebreakers only */

  gint8 min_decksize; /* IDs only */
  gint8 max_influence; /* IDs only */
  gint8 base_link; /* Runner IDs only */
};

/* Card creation functions. This is a two-step process. card_new()
   sets the common parameters of the card, then one of the card_fill_*
   functions fills in the remaining fields. */
struct card* card_new(enum faction faction,
                      const gchar *type,
                      const gchar *set,
                      gint8 number,
                      gint8 quantity,
                      gboolean unique,
                      const gchar *name,
                      const gchar *text,
                      const gchar *flavor,
                      const gchar *illustrator);

struct card* card_fill_runner_id(struct card *card,
                                 gint8 min_decksize,
                                 gint8 max_influence,
                                 gint8 base_link);
/* For events, hardware, resources and operations. */
struct card* card_fill_costed(struct card *card,
                              gboolean cost_is_x,
                              gint8 cost,
                              gint8 influence_cost);
struct card* card_fill_program(struct card *card,
                               gint8 cost,
                               gint8 influence_cost,
                               gint8 memory_cost);
struct card* card_fill_icebreaker(struct card *card,
                                  gint8 cost,
                                  gint8 influence_cost,
                                  gint8 memory_cost,
                                  gboolean strength_is_x,
                                  gint8 strength);
struct card* card_fill_corp_id(struct card *card,
                               gint8 min_decksize,
                               gint8 max_influence);
struct card* card_fill_agenda(struct card *card,
                              gint8 cost,
                              gint8 agenda_points);
struct card* card_fill_asset_upgrade(struct card *card,
                                     gint8 cost,
                                     gint8 influence_cost,
                                     gint8 trash_cost);
struct card* card_fill_ice(struct card *card,
                           gint8 cost,
                           gint8 influence_cost,
                           gint8 strength);

void card_free(struct card *card);

/* These test on the card's faction */
gboolean card_is_runner(const struct card *card);
gboolean card_is_corp(const struct card *card);
gboolean card_is_neutral(const struct card *card);

/* These test on the card's type */
gboolean card_has_agenda_points(const struct card *card);
gboolean card_has_base_link(const struct card *card);
gboolean card_has_cost(const struct card *card);
gboolean card_has_influence_cost(const struct card *card);
gboolean card_has_max_influence(const struct card *card);
gboolean card_has_memory_cost(const struct card *card);
gboolean card_has_min_decksize(const struct card *card);
gboolean card_has_strength(const struct card *card);
gboolean card_has_trash_cost(const struct card *card);

/* Return the name of the card, with a diamond in front for
   uniques. g_free() the result. */
gchar* card_render_name(const struct card *card);

#endif
