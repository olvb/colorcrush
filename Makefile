DEBUG = 0

PACKAGE = colorcrush

CC = gcc
LD = $(CC)
CFLAGS = -std=c11 -Wall -Wextra -Wno-sign-compare -Iinclude/$(PACKAGE)
LDFLAGS = -lm

ifeq ($(DEBUG), 1)
CFLAGS += -g -DDEBUG
else
CFLAGS += -O2 -DNDEBUG
endif

LIB_CFLAGS = $(CFLAGS) -fPIC
LIB_LDFLAGS = $(LDFLAGS)

DEMO_CFLAGS = $(CFLAGS) $(shell pkg-config --cflags libpng)
DEMO_LDFLAGS = $(LDFLAGS) $(shell pkg-config --libs libpng)

LIB_SRCS = $(wildcard src/*.c)
LIB_OBJS = $(patsubst src/%.c, obj/lib/%.o, $(LIB_SRCS))
LIB_DEPS = $(wildcard .d/lib/*.d)
LIB_TARGET = lib/$(PACKAGE)/lib$(PACKAGE).so

DEMO_SRCS = $(wildcard demo/*.c)
DEMO_DEPS = $(wildcard .d/demo/*.d)
DEMO_TARGETS = $(patsubst demo/%.c, bin/%, $(DEMO_SRCS))

.PHONY: all lib demo clean

all: lib demo

lib: $(LIB_TARGET)

demo: $(DEMO_TARGETS)

$(LIB_TARGET): $(LIB_OBJS)
	@mkdir -p $(@D)
	$(LD) -shared $^ -o $@ $(LIB_LDFLAGS)

$(DEMO_TARGETS): bin/%: obj/demo/%.o $(LIB_OBJS)
	@mkdir -p $(@D)
	$(LD) -o $@ $^ $(DEMO_LDFLAGS)

obj/lib/%.o: src/%.c
	@mkdir -p $(@D) .d/lib
	$(CC) $(LIB_CFLAGS) -MMD -MF .d/lib/$*.d -c -o $@ $<

obj/demo/%.o: demo/%.c
	@mkdir -p $(@D) .d/demo
	$(CC) $(DEMO_CFLAGS) -MMD -MF .d/demo/$*.d -c -o $@ $<

ifneq ($(MAKECMDGOALS), clean)
-include $(LIB_DEPS)
-include $(DEMO_DEPS)
endif

clean:
	$(RM) lib/$(PACKAGE)/* bin/* obj/lib/* obj/demo/* .d/lib/* .d/demo/*
