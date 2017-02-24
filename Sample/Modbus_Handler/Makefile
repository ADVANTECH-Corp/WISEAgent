include ../../Makefile.common

############################################################################
# Beginming of Developer Modification 
############################################################################
# for building program
application_NAME :=
# for building library
lib_NAME := Modbus_Handler
SOVERSION := 
include ../../common_version.mk

MOD_INC_DIR := $(LIB_3RD_DIR)/libmodbus-3.1.2/src
MOD_LINK_DIR := $(MOD_INC_DIR)/.libs
MSG_LIB_DIR := $(LIB_DIR)/MessageGenerator
READINI_LIB_DIR := $(LIB_DIR)/ReadINI
program_EXT_OBJS := $(PLATFORM_LINUX_DIR)/platform.o $(PLATFORM_LINUX_DIR)/common.o 
program_SUBDIRS :=
CFLAGS += -Wall  -fPIC
CPPFLAGS += -w -D_LINUX -fPIC
LDFLAGS += -Wl,-rpath,./,-lrt -lm -ldl -fPIC -shared 
program_INCLUDE_DIRS := $(PLATFORM_LINUX_DIR) $(LIB_LOG_DIR) $(LIB_SUSIIOT_DIR) $(LIB_CJSON_DIR)  $(INCLUDE_DIR) $(MOD_INC_DIR) $(MSG_LIB_DIR) $(READINI_LIB_DIR)
program_LIBRARY_DIRS := $(LIB_LOG_DIR) $(LIB_CJSON_DIR) $(INSTALL_OUTPUT_DIR)/lib $(MOD_LINK_DIR) $(MSG_LIB_DIR) $(READINI_LIB_DIR)
program_LIBRARIES := Log cJSON pthread modbus msggen readini
############################################################################
# End of Developer Modification 
############################################################################

ifneq ($(strip $(ROOTFS)),)
program_INCLUDE_DIRS += $(ROOTFS)/usr/include
program_LIBRARY_DIRS += $(ROOTFS)/usr/lib
LDFLAGS += -Wl,--rpath-link $(ROOTFS)/usr/lib
endif
ifneq ($(strip $(lib_NAME)),)
#library_STATIC := $(lib_NAME).a
library_SO := $(lib_NAME).so
library_CLEANTARGET := $(library_STATIC) $(library_SO)*
ifneq ($(strip $(SOVERSION)),)
library_DYNAMIC := $(lib_NAME).so.$(SOVERSION) 
else
library_DYNAMIC := $(library_SO)
endif
library_NAME := $(library_STATIC) $(library_DYNAMIC)
INSTALL_OUTPUT_DIR := $(INSTALL_OUTPUT_DIR)/module
endif
program_C_SRCS := $(wildcard *.c)
program_CXX_SRCS := $(wildcard *.cpp)
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_CXX_OBJS := ${program_CXX_SRCS:.cpp=.o}
program_OBJS := $(program_C_OBJS) $(program_CXX_OBJS) $(program_EXT_OBJS)
CPPFLAGS += $(foreach includedir,$(program_INCLUDE_DIRS),-I$(includedir))
LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))

.PHONY: all clean distclean install

all: $(application_NAME) $(library_NAME)

$(application_NAME): $(program_OBJS) $(program_SUBDIRS)
	$(LINK.cc) $(program_OBJS) -o $@ $(LDFLAGS)

$(library_STATIC): $(program_OBJS) $(program_SUBDIRS)
	$(AR) cr $@ $^

$(library_DYNAMIC): $(program_OBJS) $(program_SUBDIRS)
	$(CC) -shared $^ -o $@ $(LDFLAGS)
ifneq ($(strip $(SOVERSION)),)
	ln -s $(library_DYNAMIC) $(library_SO)
endif

$(program_SUBDIRS):
	$(MAKE) -C $@

clean:
	@- $(RM) $(application_NAME) $(library_CLEANTARGET)
	@- $(RM) $(program_OBJS)

distclean: clean
	@- $(RM) $(INSTALL_OUTPUT_DIR)/$(library_SO)

install: 
	@- mkdir -p $(INSTALL_OUTPUT_DIR)
	cp -d $(library_CLEANTARGET) $(INSTALL_OUTPUT_DIR)
