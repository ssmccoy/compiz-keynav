libkeynav_la_LDFLAGS = -module -avoid-version -no-undefined
libkeynav_la_LIBADD = @COMPIZ_LIBS@
libkeynav_la_SOURCES = keynav.c

INCLUDES = @COMPIZ_CFLAGS@

moduledir = $(plugindir)

module_LTLIBRARIES = libkeynav.la

XML_FILES = keynav.xml

config_DATA = $(XML_FILES)
configdir = $(datadir)/compiz

EXTRA_DIST = $(XML_FILES)
