# Target library
lib = libfs.a
objs = disk.o fs.o

AR = ar
ARFLAGS = rcs
CC = gcc
CFLAGS = -Wall -Wextra -Werror -MMD

# Debug flag
ifneq ($(D),1)
CFLAGS += -O2
else
CFLAGS += -g
endif

#Print all commands flag
ifneq ($(V),1)
Q=@
endif

all: $(lib)

deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

$(lib): $(objs)
	@echo "AR $@"
	$(Q)$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	@echo "clean"
	$(Q)rm -f $(lib) $(objs) $(deps)