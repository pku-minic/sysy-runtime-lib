# parameters
NO_LIBC ?= 0
ADD_CFLAGS ?=

# C compiler
CFLAGS := -O3 -c -Wall -Werror
ifneq ($(NO_LIBC), 0)
CFLAGS += -DNO_LIBC
endif
CC := clang $(CFLAGS) $(ADD_CFLAGS)

# archiver
ARFLAGS := ru
AR := llvm-ar $(ARFLAGS)

# ranlib
RANLIBFLAGS :=
RANLIB := llvm-ranlib $(RANLIBFLAGS)

# directories
TOP_DIR := $(shell pwd)
SRC_DIR := $(TOP_DIR)/src
BUILD_DIR := $(TOP_DIR)/build
OBJ_DIR := $(BUILD_DIR)/obj

# files
SRCS := $(SRC_DIR)/sysy.c
ifneq ($(NO_LIBC), 0)
SRCS += $(shell find $(SRC_DIR)/nolibc -name "*.c")
endif
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# targets
LIBSYSY := $(BUILD_DIR)/libsysy.a


$(LIBSYSY): $(OBJS)
	mkdir -p $(dir $@)
	$(AR) $@ $^
	$(RANLIB) $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $^ -o $@ -I$(SRC_DIR)

.PHONY: clean
clean:
	-rm -rf $(OBJ_DIR)
	-rm $(LIBSYSY)
