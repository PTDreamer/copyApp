# Makefile for copyApp project
.DEFAULT_GOAL := help

WHEREAMI := $(dir $(lastword $(MAKEFILE_LIST)))
ROOT_DIR := $(realpath $(WHEREAMI)/ )

# configure some directories that are relative to wherever ROOT_DIR is located
BUILD_DIR := $(ROOT_DIR)/build

##############################
#
# Set up paths to tools
#
##############################
QT_SDK_DIR=/home/jose/Code/TauLabs/tools/Qt5.5.0
QT_SDK_QMAKE_PATH=/home/jose/Code/TauLabs/tools/Qt5.5.0/5.5/gcc_64/bin/qmake
ifeq ($(shell [ -d "$(QT_SDK_DIR)" ] && echo "exists"), exists)
  QMAKE = $(QT_SDK_QMAKE_PATH)
ifdef WINDOWS
  # Windows needs to be told where to find Qt libraries
  export PATH := $(QT_SDK_DIR)/5.5/mingw492_32/bin:$(PATH)
endif
else
  # not installed, hope it's in the path...
endif

# make sure this isn't being run as root, not relevant for windows
ifndef WINDOWS
  # Deal with unreasonable requests
  # See: http://xkcd.com/149/
  ifeq ($(MAKECMDGOALS),me a sandwich)
    ifeq ($(shell whoami),root)
      $(error Okay)
    else
      $(error What? Make it yourself)
    endif
  endif

  # Seriously though, you shouldn't ever run this as root
  ifeq ($(shell whoami),root)
    $(error You should not be running this as root)
  endif
endif

# Function for converting an absolute path to one relative
# to the top of the source tree.
toprel = $(subst $(realpath $(ROOT_DIR))/,,$(abspath $(1)))

# Clean out undesirable variables from the environment and command-line
# to remove the chance that they will cause problems with our build
define SANITIZE_VAR
$(if $(filter-out undefined,$(origin $(1))),
  $(info *NOTE*      Sanitized $(2) variable '$(1)' from $(origin $(1)))
  MAKEOVERRIDES = $(filter-out $(1)=%,$(MAKEOVERRIDES))
  override $(1) :=
  unexport $(1)
)
endef

# These specific variables can influence gcc in unexpected (and undesirable) ways
SANITIZE_GCC_VARS := TMPDIR GCC_EXEC_PREFIX COMPILER_PATH LIBRARY_PATH
SANITIZE_GCC_VARS += CFLAGS CPATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH OBJC_INCLUDE_PATH DEPENDENCIES_OUTPUT
SANITIZE_GCC_VARS += ARCHFLAGS
$(foreach var, $(SANITIZE_GCC_VARS), $(eval $(call SANITIZE_VAR,$(var),disallowed)))

# Decide on a verbosity level based on the V= parameter
export AT := @

ifndef V
export V0    :=
export V1    := $(AT)
else ifeq ($(V), 0)
export V0    := $(AT)
export V1    := $(AT)
else ifeq ($(V), 1)
endif

# copyApp build configuration (debug | release)
BUILD_CONF ?= release

# Checking for $(BUILD_CONF) to be sane
ifdef BUILD_CONF
 ifneq ($(BUILD_CONF), release)
  ifneq ($(BUILD_CONF), debug)
   $(error Only debug or release are allowed for BUILD_CONF)
  endif
 endif
endif

##############################
#
# Help instructions
#
##############################
.PHONY: help
help:
	@echo
	@echo "   This Makefile is known to work on Linux and Mac in a standard shell environment."
	@echo
	@echo "   Here is a summary of the available targets:"
	@echo
	@echo "   [Tool Installers]"
	@echo "     app       	- Build and package the application"
	@echo "     clean	- Remove the application"
	@echo
	@echo
	@echo "   Hint: Add V=1 to your command line to see verbose build output."
	@echo
	@echo "         All build output will be placed in $(BUILD_DIR)"
	@echo

$(BUILD_DIR):
	mkdir -p $@

##############################
#
# TARGETS
#
##############################

ifndef WINDOWS
# unfortunately the silent linking command is broken on windows
ifeq ($(V), 1)
SILENT := 
else
SILENT := silent
endif
endif

.PHONY: package
package: app
	$(V1) mkdir -p $(BUILD_DIR)/$@/lib
	$(V1)find $(BUILD_DIR)/copyApp -exec ldd {} \; \
	| sed -e '/^[^\t]/ d; s/^\t\(.* => \)\?\([^ ]*\) (.*/\2/g'| grep "Qt5" | grep -v "build" | sort | uniq | xargs -I '{}' cp -v -f '{}' $(BUILD_DIR)/package/lib
	$(V1) cp -v -f $(ROOT_DIR)/copyApp $(BUILD_DIR)/package/
	$(V1) cp -v -f $(BUILD_DIR)/copyApp.bin $(BUILD_DIR)/package/
	$(V1)tar -cvJf $(BUILD_DIR)/package.tar.xz -C $(BUILD_DIR) package
.PHONY: app
app:
	$(V1) mkdir -p $(BUILD_DIR)
	$(V1) ( cd $(BUILD_DIR) && \
	  $(QMAKE) $(ROOT_DIR)/copyApp.pro CONFIG+="$(BUILD_CONF)" $(QMAKE_OPTS) && \
	  $(MAKE) -w ; \
	)

.PHONY: clean
clean:
	$(V0) @echo " CLEAN"
	$(V1) [ ! -d "$(BUILD_DIR)" ] || $(RM) -r "$(BUILD_DIR)"
