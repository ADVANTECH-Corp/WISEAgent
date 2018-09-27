AM_CPPFLAGS = -Wall -fPIC -I$(top_srcdir)/Platform -I$(top_srcdir)/Platform/Linux/ -I$(top_srcdir)/Include -I$(top_srcdir)/Library/cJSON -I$(top_srcdir)/Library/Log -I$(top_srcdir)/Library/ReadINI -I$(top_srcdir)/Library/MessageGenerator -I$(top_srcdir)/Library/HandlerKernel -I$(top_srcdir)/Library3rdParty/libmodbus-3.1.2/src
lib_LTLIBRARIES = ModbusSample.la
ModbusSample_la_SOURCES = Modbus_Handler.cpp Modbus_Handler.h Modbus_HandlerLog.h Modbus_Parser.c Modbus_Parser.h
ModbusSample_la_LDFLAGS = -L$(top_srcdir)/Platform -lWISEPlatform -lWISEUtil -L$(top_srcdir)/Library/cJSON -lcJSON -L$(top_srcdir)/Library/Log -lLog -L$(top_srcdir)/Library/ReadINI -lReadINI -L$(top_srcdir)/Library/MessageGenerator -lmsggen -L$(top_srcdir)/Library/HandlerKernel -lHandlerKernel -L$(top_srcdir)/Library3rdParty/libmodbus-3.1.2/src/.libs -lmodbus -lpthread -ldl -release $(PACKAGE_VERSION_MAJOR).$(PACKAGE_VERSION_MINOR).$(PACKAGE_VERSION_PATCH).$(PACKAGE_VERSION_REVISION) -module
LDADD = $(INTLLIBS)
