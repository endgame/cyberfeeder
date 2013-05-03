## Process this file with automake to generate Makefile.in
ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = $(GTK3_CFLAGS) $(Jansson_CFLAGS)
AM_CPPFLAGS = -I$(srcdir)/src -DDEFAULT_DATA_DIR=\"$(pkgdatadir)\"
LDADD = $(GTK3_LIBS) $(Jansson_LIBS)

cardsdir = $(pkgdatadir)/cards
dist_cards_DATA = \
	data/cards/00_Core.json \
	data/cards/01_What_Lies_Ahead.json \
	data/cards/02_Trace_Amount.json \
	data/cards/03_Cyber_Exodus.json

bin_PROGRAMS = cyberfeeder
CLEANFILES = src/hash_card_type.c src/hash_faction_name.c

cyberfeeder_SOURCES = \
	src/card.c \
	src/card.h \
	src/card_db.c \
	src/card_db.h \
	src/card_set.c \
	src/card_set.h \
	src/cyberfeeder.c \
	src/hash.h \
	src/hash_card_type.gperf \
	src/hash_faction_name.gperf \
	src/pages.h \
	src/pages/the_toolbox.c \
	src/ui_helpers.c \
	src/ui_helpers.h

.gperf.c:
	$(AM_V_GEN)$(GPERF) $< > $@
