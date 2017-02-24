AM_CPPFLAGS = -Wall -fPIC -I$(top_srcdir)/Library/cJSON -I$(top_srcdir)/Include -I$(top_srcdir)/Platform -I$(top_srcdir)/Platform/Linux
lib_LIBRARIES = libmsggen.a
libmsggen_a_SOURCES = MsgGenerator.c DeviceMessageGenerate.c GPSMessageGenerate.c IoTMessageGenerate.c
AM_LDFLAGS = -L$(top_srcdir)/Library/cJSON -lcJSON -ldl -release $(PACKAGE_VERSION_MAJOR).$(PACKAGE_VERSION_MINOR).$(PACKAGE_VERSION_PATCH).$(PACKAGE_VERSION_REVISION)
