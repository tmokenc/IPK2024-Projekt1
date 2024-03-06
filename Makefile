PROJ=ipk24chat-client
SRC_DIR=src
DOC_DIR=doc
LIB_DIR=lib
BUILD_DIR=build

CC=gcc
CFLAGS=-Wall -Wextra -O2 -MMD -Werror -Wpedantic -g
# LDFLAGS=

SRCS=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.d,$(SRCS))

LIBS=$(wildcard $(LIB_DIR)/*.c)
LIB_OBJS=$(patsubst $(LIB_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIBS))
LIB_DEPS=$(patsubst $(LIB_DIR)/%.c,$(BUILD_DIR)/%.d,$(LIBS))

DOCS=$(wildcard $(DOC_DIR)/*)

$(PROJ): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

test: build/args.o build/error.o build/payload.o build/bytes.o build/error.o
	$(CC) $(CFLAGS) -o build/test test/main.c $^ && \
	./build/test -v

doc: $(DOCS) doc/refs.bib doc/state_machine.dot
	dot -Tpng doc/state_machine.dot -o doc/state_machine.png
	typst c doc/documentation.typ documentation.pdf

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)
-include $(LIB_DEPS)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(PROJ)
