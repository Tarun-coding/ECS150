# Target library
lib := libuthread.a
objs := queue.o uthread.o context.o preempt.o

AR := ar
ARFLAGS := rcs
CC := gcc
CFLAGS := -Wall -Wextra -Werror -MMD
# Debug flag
ifneq ($(D),1)
CFLAGS	+= -O2
else
CFLAGS	+= -g
endif

# Print all commands flag
ifneq ($(V),1)
Q=@
endif



all: $(lib)

deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

libuthread.a: $(objs)
	@echo "AR $@"
	$(Q)$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	@echo "clean"
	$(Q)rm -f $(lib) $(objs) $(deps)