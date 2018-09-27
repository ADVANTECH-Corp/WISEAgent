ifneq ($(strip $(ROOTFS)),)
program_INCLUDE_DIRS += $(ROOTFS)/usr/include
program_LIBRARY_DIRS += $(ROOTFS)/usr/lib
LDFLAGS += -Wl,--rpath-link $(ROOTFS)/usr/lib
endif
ifneq ($(strip $(lib_NAME)),)
library_STATIC := $(lib_NAME).a
library_SO := $(lib_NAME).so
library_CLEANTARGET := $(library_STATIC) $(library_SO)*
ifneq ($(strip $(SOVERSION)),)
library_DYNAMIC := $(lib_NAME).so.$(SOVERSION) 
else
library_DYNAMIC := $(library_SO)
endif
library_NAME := $(if $(filter $(shared_lib_NAME), $(lib_NAME)), \
  $(library_STATIC) $(library_DYNAMIC), \
  $(library_STATIC))
INSTALL_OUTPUT_DIR := $(INSTALL_OUTPUT_DIR)
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
	$(LINK.cc) -shared $^ -o $@ $(LDFLAGS)
ifneq ($(strip $(SOVERSION)),)
	ln -s $(library_DYNAMIC) $(library_SO)
endif

$(program_SUBDIRS):
	$(MAKE) -C $@

clean:
	@- $(RM) $(application_NAME) $(library_CLEANTARGET)
	@- $(RM) $(program_OBJS)
ifneq ($(strip $(application_NAME)),)
	@- $(RM) $(INSTALL_OUTPUT_DIR)/$(application_NAME)
endif
ifneq ($(strip $(lib_NAME)),)
	@- $(RM) $(INSTALL_OUTPUT_DIR)/$(library_CLEANTARGET)
endif

distclean: clean

install: 
	@- cp -d $(application_NAME) $(library_DYNAMIC) $(library_SO) $(INSTALL_OUTPUT_DIR)

