include ../../common_config.mk
include ../../common_version.mk

############################################################################
# Beginming of Developer Modification 
############################################################################
# for building program
application_NAME := 
# for building library
lib_NAME := libSAManager

CJSON_INC_DIR :=$(LIB_DIR)/cJSON
LOG_INC_DIR := $(LIB_DIR)/Log


program_EXT_OBJS := $(PLATFORMS_DIR)/basequeue.o $(PLATFORM_LINUX_DIR)/util_path.o $(PLATFORM_LINUX_DIR)/util_string.o $(PLATFORM_LINUX_DIR)/util_libloader.o $(PLATFORM_LINUX_DIR)/NamedPipeClient.o

program_SUBDIRS :=
CFLAGS += -Wall -fPIC
CPPFLAGS += -D_LINUX
LDFLAGS += 
program_INCLUDE_DIRS := $(PLATFORMS_DIR) $(PLATFORM_LINUX_DIR) $(INCLUDE_DIR) $(CJSON_INC_DIR) $(LOG_INC_DIR) $(LIB_DIR)/SAConfig $(ROOTFS)/usr/include/libxml2
program_LIBRARY_DIRS := $(PROJECT_ROOT_DIR) $(LOG_INC_DIR) $(CJSON_INC_DIR) $(LIB_DIR)/SAConfig
program_LIBRARIES := cJSON Log SAConfig xml2 pthread dl
############################################################################
# End of Developer Modification 
############################################################################

include ../../common_rule.mk

