
CFLAGS?=
CFLAGS+= -std=c++1y -I../include
CXX:= c++

clean:
	rm -rf *.exe

define therule
.PHONY: $(1)

$(1): $(1).exe

$(1).exe: $(1).cpp ../include/fusion_includes.h ../include/stat_log.h
	c++ $(CFLAGS) -o $$@ $$<


endef

ifneq ($(MAKECMDGOALS),clean)
$(eval $(call therule, $(MAKECMDGOALS)))
endif


