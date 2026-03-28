# parameters
NO_LIBC ?= 0
ADD_CFLAGS ?=
ADD_LDFLAGS ?=
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
ARFLAGS := rc
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
BUILT_TEST_DIR := $(BUILD_DIR)/test

# files
SRCS := $(SRC_DIR)/sysy.c
ifneq ($(NO_LIBC), 0)
SRCS += $(shell find $(SRC_DIR)/nolibc -name "*.c")
endif
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
TEST_SRCS := $(shell find $(TEST_DIR) -name "*.c")

# targets
LIBSYSY := $(BUILD_DIR)/libsysy.a
TESTS := $(patsubst $(TEST_DIR)/%.c, $(BUILT_TEST_DIR)/%, $(TEST_SRCS))


.PHONY: libsysy test clean

libsysy: $(LIBSYSY)

test: $(TESTS)

clean:
	-rm -rf $(OBJ_DIR)
	-rm -rf $(BUILT_TEST_DIR)
	-rm $(LIBSYSY)

FORCE: ;

$(LIBSYSY): $(OBJS)
	mkdir -p $(dir $@)
	$(AR) $@ $^
	$(RANLIB) $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $^ -o $@ -I$(SRC_DIR) -c $(LIB_OPT)

$(BUILT_TEST_DIR)/%: $(TEST_DIR)/%.c $(LIBSYSY)
	mkdir -p $(dir $@)
	$(CC) $(ADD_LDFLAGS) $< -o $@ -I$(SRC_DIR) -L$(BUILD_DIR) -lsysy $(TEST_OPT)
