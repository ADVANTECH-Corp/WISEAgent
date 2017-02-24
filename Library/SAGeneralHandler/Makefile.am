AM_CPPFLAGS = -Wall -fPIC -I$(top_srcdir)/Platform -I$(top_srcdir)/Platform/Linux/ -I$(top_srcdir)/Include -I$(top_srcdir)/Library/SAConfig -I$(top_srcdir)/Library/Log -I$(top_srcdir)/Library/FtpHelper -I$(top_srcdir)/Library/MD5 -I$(top_srcdir)/Library/cJSON $(XML_CFLAGS)
lib_LTLIBRARIES = libSAGeneralHandler.la
libSAGeneralHandler_la_SOURCES = agentupdater.c generallog.h resource.h version.h
				 agentupdater.h ghparser.c SAGeneralHandler.c
				 general_def.h ghparser.h SAGeneralHandler.h
				 ../../Platform/Linux/util_process.c ../../Platform/Linux/util_path.c
libSAGeneralHandler_la_LDFLAGS = $(XML_LIBS) $(CURL_LIBS) -L$(top_srcdir)/Library/Log -lLog -L$(top_srcdir)/Library/FtpHelper -lftphelper -L$(top_srcdir)/Library/MD5 -lmd5 -L$(top_srcdir)/Library/cJSON -lcJSON -lpthread -ldl -release $(PACKAGE_VERSION_MAJOR).$(PACKAGE_VERSION_MINOR).$(PACKAGE_VERSION_PATCH).$(PACKAGE_VERSION_REVISION)
LDADD = $(top_srcdir)/Library/SAConfig/libSAConfig.la $(INTLLIBS)
