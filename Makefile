PROJ=ipk24chat-client
SRC_DIR=src
BUILD_DIR=build

CFLAGS=-Wall -Wextra -O2 -MMD -Werror -Wpedantic -g $(DEFINES)
# LDFLAGS=

SRCS=$(wildcard $(SRC_DIR)/*.c)
TEST=$(wildcard $(TEST_DIR)/*.c)
OBJS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.d,$(SRCS))
TEST_EXECUTABLES = $(patsubst $(TEST_DIR)/%.c, %, $(TEST))
TEST_OBJS = $(subst main.o,, $(OBJS))

$(PROJ): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

doc:
	typst c doc/documentation.typ documentation.pdf

-include $(DEPS)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(PROJ)
