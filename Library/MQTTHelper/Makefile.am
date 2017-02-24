ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -Wall -fPIC -I$(top_srcdir)/Platform/Linux/ -I$(top_srcdir)/Include
lib_LTLIBRARIES = libmqtthelper.la
libmqtthelper_la_SOURCES = mqtthelper.c ../../Platform/Linux/util_path.c
libmqtthelper_la_LDFLAGS = -lmosquitto -lpthread -ldl -release $(PACKAGE_VERSION_MAJOR).$(PACKAGE_VERSION_MINOR).$(PACKAGE_VERSION_PATCH).$(PACKAGE_VERSION_REVISION)

