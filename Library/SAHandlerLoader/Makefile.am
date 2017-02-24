AM_CPPFLAGS = -Wall -fPIC -I$(top_srcdir)/Platform -I$(top_srcdir)/Platform/Linux/ -I$(top_srcdir)/Include -I$(top_srcdir)/Library/Log $(XML_CFLAGS)
lib_LTLIBRARIES = libSAHandlerLoader.la
libSAHandlerLoader_la_SOURCES = moduleconfig.c SALoader.c ../../Platform/Linux/util_path.c ../../Platform/Linux/util_libloader.c
libSAHandlerLoader_la_LDFLAGS = $(XML_LIBS) -L$(top_srcdir)/Library/Log -lLog -lpthread -ldl -release $(PACKAGE_VERSION_MAJOR).$(PACKAGE_VERSION_MINOR).$(PACKAGE_VERSION_PATCH).$(PACKAGE_VERSION_REVISION)
include_HEADERS = $(top_srcdir)/Library/SAHandlerLoader/SALoader.h
LDADD = $(top_srcdir)/Library/SAConfig/libSAConfig.la $(INTLLIBS)
