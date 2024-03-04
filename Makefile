PROJ=ipk24chat-client
SRC_DIR=src
BUILD_DIR=build

CC=gcc
CFLAGS=-Wall -Wextra -O2 -MMD -Werror -Wpedantic -g
# LDFLAGS=

SRCS=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.d,$(SRCS))

$(PROJ): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

test: build/args.o build/error.o build/payload.o build/bytes.o
	$(CC) $(CFLAGS) -o build/test test/main.c $^ && \
	./build/test -v

doc:
	typst c doc/documentation.typ documentation.pdf

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)
-include $(TEST_DEPS)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(PROJ)
