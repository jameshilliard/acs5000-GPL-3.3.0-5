ifeq ($(filter $(TARG), cyc acs5k),$(TARG))
TARGETS = build_acs clean
else ifeq ($(filter $(TARG), apc kvmapc),$(TARG))
TARGETS = build_apc clean
else
TARGETS = build clean
endif

JAVA_MINDTERM=/usr/java/j2sdk1.4.2_04

# Default target
.PHONY: ${TARGETS}

${TARGETS}:
	JAVA_HOME=$(JAVA_MINDTERM) PATH=$(JAVA_MINDTERM)/bin:$(PATH) ant $@
#	cp build/$(PKGNAME).jar $(APPLDIR)/$(PKGNAME).jar
