include ../../common_config.mk
include ../../common_version.mk

############################################################################
# Beginming of Developer Modification 
############################################################################
# for building program
application_NAME := 
# for building library
lib_NAME := libSAGeneralHandler

XML2_INC_DIR := $(ROOTFS)/usr/include/libxml2
LOG_INC_DIR := $(LIB_DIR)/Log
FTPHELPER_INC_DIR := $(LIB_DIR)/FtpHelper
MD5_INC_DIR := $(LIB_DIR)/MD5
SACONFIG_INC_DIR := $(LIB_DIR)/SAConfig
CJSON_INC_DIR := $(LIB_DIR)/cJSON

program_EXT_OBJS := $(PLATFORM_LINUX_DIR)/util_process.o $(PLATFORM_LINUX_DIR)/util_path.o
program_SUBDIRS :=
CFLAGS += -Wall -fPIC
CPPFLAGS += -D_LINUX -DLOG4Z
LDFLAGS += -Wl,-rpath,./
program_INCLUDE_DIRS := $(PLATFORMS_DIR) $(PLATFORM_LINUX_DIR) $(INCLUDE_DIR) $(XML2_INC_DIR) $(LOG_INC_DIR) $(FTPHELPER_INC_DIR) $(MD5_INC_DIR) $(SACONFIG_INC_DIR) $(CJSON_INC_DIR)
program_LIBRARY_DIRS := $(PROJECT_ROOT_DIR) $(LOG_INC_DIR) $(FTPHELPER_INC_DIR) $(MD5_INC_DIR) $(SACONFIG_INC_DIR) $(CJSON_INC_DIR)
program_LIBRARIES := xml2 cJSON Log ftphelper curl md5 SAConfig pthread dl
############################################################################
# End of Developer Modification 
############################################################################

include ../../common_rule.mk

