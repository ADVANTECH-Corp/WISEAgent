AM_CPPFLAGS = -Wall -fPIC -I$(top_srcdir)/Platform -I$(top_srcdir)/Platform/Linux -I$(top_srcdir)/Include -I$(top_srcdir)/Library/cJSON -I$(top_srcdir)/Library/MessageGenerator
lib_LTLIBRARIES = HandlerSample.la
HandlerSample_la_SOURCES = handler_api.c
HandlerSample_la_LDFLAGS = -L$(top_srcdir)/Library/cJSON -lcJSON -L$(top_srcdir)/Library/MessageGenerator -lmsggen -lpthread -ldl -release $(PACKAGE_VERSION_MAJOR).$(PACKAGE_VERSION_MINOR).$(PACKAGE_VERSION_PATCH).$(PACKAGE_VERSION_REVISION) -module
