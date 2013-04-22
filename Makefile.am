## Process this file with automake to generate Makefile.in
ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = $(GTK3_CFLAGS) $(Jansson_CFLAGS)
AM_CPPFLAGS = -I$(srcdir)/src
LDADD = $(GTK3_LIBS) $(Jansson_LIBS)

bin_PROGRAMS = cyberfeeder
CLEANFILES = src/faction_lookup.c
EXTRA_DIST = data/00_Core.json

cyberfeeder_SOURCES = \
	src/card.c \
	src/card.h \
	src/card_db.c \
	src/card_db.h \
	src/cyberfeeder.c \
	src/faction_lookup.gperf \
	src/faction_lookup.h

.gperf.c:
	$(AM_V_GEN)$(GPERF) $< > $@
