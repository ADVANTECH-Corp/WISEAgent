ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -Wall -fPIC -I$(top_srcdir)/Platform/Linux/ -I$(top_srcdir)/Include -I$(top_srcdir)/Library/Log -I$(top_srcdir)/Library/SAConfig -I$(top_srcdir)/Library/SAClient
bin_PROGRAMS = sampleagent
sampleagent_SOURCES = sampleagent.c ../../Platform/Linux/platform.c ../../Platform/Linux/common.c
sampleagent_LDFLAGS = -lpthread -ldl -release $(PACKAGE_VERSION_MAJOR).$(PACKAGE_VERSION_MINOR).$(PACKAGE_VERSION_PATCH).$(PACKAGE_VERSION_REVISION)
LDADD = -lstdc++ $(top_srcdir)/Library/SAClient/libSAClient.la $(top_srcdir)/Library/SAConfig/libSAConfig.la $(top_srcdir)/Library/Log/libLog.a $(INTLLIBS)