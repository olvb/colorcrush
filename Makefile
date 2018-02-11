CC = gcc
LD = $(CC)
CFLAGS = -std=c11 -Wall -Wextra -Wno-sign-compare
CFLAGS_DBG = -g -DDEBUG
CFLAGS_RLS = -O2 -DNDEBUG
ifeq ($(DEBUG), 1)
	CFLAGS += $(CFLAGS_DBG)
else
	CFLAGS += $(CFLAGS_RLS)
endif
LDFLAGS = -lpng -lz -lm

TARGET = colorcrush

SRC_DIR = src
OBJ_DIR = obj
DEP_DIR = .d
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
HEADER_FILES = $(wildcard $(SRC_DIR)/*.h)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))
DEP_FILES = $(patsubst $(SRC_DIR)/%.c, $(DEP_DIR)/%.d, $(SRC_FILES))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@mkdir -p $(@D)
	$(LD) -o $@ $(OBJ_FILES) $(LDFLAGS)

$(OBJ_DIR)/%.o:
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(DEP_DIR)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MM -MT $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $<) $< -MF $@

ifneq ($(MAKECMDGOALS), clean)
    -include $(DEP_FILES)
endif

clean:
	$(RM) $(OBJ_FILES) $(DEP_FILES) $(TARGET) .deps
