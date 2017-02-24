include ../../common_config.mk
include ../../common_version.mk

############################################################################
# Beginming of Developer Modification 
############################################################################
# for building program
application_NAME := 
# for building library
lib_NAME := libSAHandlerLoader

XML2_INC_DIR := $(ROOTFS)/usr/include/libxml2
LOG_INC_DIR := $(LIB_DIR)/Log


program_EXT_OBJS := $(PLATFORM_LINUX_DIR)/util_path.o $(PLATFORM_LINUX_DIR)/util_libloader.o
program_SUBDIRS :=
CFLAGS += -Wall -fPIC
CPPFLAGS += -D_LINUX
LDFLAGS += -Wl,-rpath,./
program_INCLUDE_DIRS := $(PLATFORMS_DIR) $(PLATFORM_LINUX_DIR) $(INCLUDE_DIR) $(XML2_INC_DIR) $(LOG_INC_DIR)
program_LIBRARY_DIRS := $(PROJECT_ROOT_DIR) $(LOG_INC_DIR) $(LIB_DIR)/SAConfig
program_LIBRARIES := xml2 Log SAConfig pthread dl
############################################################################
# End of Developer Modification 
############################################################################

include ../../common_rule.mk

