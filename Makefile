include common_config.mk
include common_version.mk

ifneq ($(strip $(ROOTFS)),)
LDFLAGS += -L$(ROOTFS)/usr/lib
LDFLAGS += -Wl,--rpath-link $(ROOTFS)/usr/lib
endif


.PHONY: all clean distclean install $(application_DIR) $(application_LIBSDIR) $(module_LIBDIR)

all: $(application_DIR) $(module_LIBDIR)

$(application_DIR): $(application_LIBSDIR)
	$(MAKE) -C $@

$(application_LIBSDIR):
	$(MAKE) -C $@
	
$(module_LIBDIR): $(application_LIBSDIR)
	$(MAKE) -C $@

clean:
	for d in $(application_DIR) $(application_LIBSDIR) $(module_LIBDIR); \
	do \
		$(MAKE) -C $$d clean; \
	done
	@- $(RM) $(core_lib_CLEANTARGET)
	@- $(RM) $(INSTALL_OUTPUT_DIR)/lib/*
	@- $(RM) $(INSTALL_OUTPUT_DIR)/module/*.so*

distclean: clean

install:
	mkdir -p $(INSTALL_OUTPUT_DIR)
	mkdir -p $(INSTALL_OUTPUT_DIR)/lib
ifeq ($(strip $(core_lib_NAME)),)
	for d in $(application_DIR) $(application_LIBSDIR) $(module_LIBDIR); \
	do \
		$(MAKE) -C $$d install; \
	done
else
	for d in $(application_DIR) $(module_LIBDIR) $(shared_LIBDIR); \
	do \
		$(MAKE) -C $$d install; \
	done
	mv $(core_lib_CLEANTARGET) $(INSTALL_OUTPUT_DIR)/lib
endif

