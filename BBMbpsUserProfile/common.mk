ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

USEFILE=

include $(MKFILES_ROOT)/qmacros.mk

# Suppress the _g suffix from the debug variant
BUILDNAME=$(IMAGE_PREF_$(BUILD_TYPE))$(NAME)$(IMAGE_SUFF_$(BUILD_TYPE))

# Extra include path libfreetype and for target overrides and patches
EXTRA_INCVPATH+=$(QNX_TARGET)/usr/include/freetype2 \
	$(QNX_TARGET)/../target-override/usr/include

# Extra library search path for target overrides and patches
EXTRA_LIBVPATH+=$(QNX_TARGET)/../target-override/$(CPUVARDIR)/lib \
	$(QNX_TARGET)/../target-override/$(CPUVARDIR)/usr/lib

EXTRA_LIBVPATH += $(QNX_TARGET)/$(CPU)/usr/lib/qt4/lib
EXTRA_LIBVPATH += $(QNX_TARGET)/armle-v7/usr/lib/qt4/lib

# Compiler options for enhanced security
CCFLAGS+=-fstack-protector-all -D_FORTIFY_SOURCE=2 \
	$(if $(filter g so shared,$(VARIANTS)),,-fPIE)

CCFLAGS += -Wformat -Wformat-security -Werror=format-security

# And, of course, enable all normal warnings
CCFLAGS += -Wall

CCFLAGS += -Werror
  
CCFLAGS += -fno-strict-aliasing

# Linker options for enhanced security
LDFLAGS+=-Wl,-z,relro -Wl,-z,now $(if $(filter g so shared,$(VARIANTS)),,-pie)

# Basic libraries required by most native applications

include $(MKFILES_ROOT)/qtargets.mk

LIBS+=bps bbmsp QtCore cpp

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

-include $(PROJECT_ROOT)/../samples.mk
