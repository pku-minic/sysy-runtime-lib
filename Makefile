# parameters
NO_LIBC ?= 0
ADD_CFLAGS ?=
LIB_OPT ?= -O3
TEST_OPT ?= -O1

# C compiler
CFLAGS := -Wall -Werror -Wno-unused-result
ifneq ($(NO_LIBC), 0)
CFLAGS += -DNO_LIBC
ifneq ($(shell uname), Darwin)
CFLAGS += -nostdlib -nostdinc -static
endif
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
TEST_DIR := $(TOP_DIR)/test
BUILD_DIR := $(TOP_DIR)/build
OBJ_DIR := $(BUILD_DIR)/obj

# files
SRCS := $(SRC_DIR)/sysy.c
ifneq ($(NO_LIBC), 0)
SRCS += $(shell find $(SRC_DIR)/nolibc -name "*.c")
endif
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
TEST_SRC := $(TEST_DIR)/test.c

# targets
LIBSYSY := $(BUILD_DIR)/libsysy.a
TEST := $(BUILD_DIR)/test


.PHONY: libsysy test clean

libsysy: $(LIBSYSY)

test: $(TEST)

clean:
	-rm -rf $(OBJ_DIR)
	-rm $(LIBSYSY)

$(LIBSYSY): $(OBJS)
	mkdir -p $(dir $@)
	$(AR) $@ $^
	$(RANLIB) $@

$(TEST): $(LIBSYSY) $(TEST_SRC)
	$(CC) $(TEST_SRC) -o $@ -I$(SRC_DIR) -L$(BUILD_DIR) -lsysy $(TEST_OPT)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $^ -o $@ -I$(SRC_DIR) -c $(LIB_OPT)
