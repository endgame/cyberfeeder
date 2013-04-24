## Process this file with automake to generate Makefile.in
ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = $(GTK3_CFLAGS) $(Jansson_CFLAGS)
AM_CPPFLAGS = -I$(srcdir)/src
LDADD = $(GTK3_LIBS) $(Jansson_LIBS)

bin_PROGRAMS = cyberfeeder
CLEANFILES = src/hash_card_type.c src/hash_faction_name.c
EXTRA_DIST = data/00_Core.json

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
	src/hash_faction_name.gperf

.gperf.c:
	$(AM_V_GEN)$(GPERF) $< > $@
