PROJ=ipk24chat-client
SERVER=ipk24chat-server
SRC_DIR=src
DOC_DIR=doc
BUILD_DIR=build

CC=gcc
CFLAGS=-Wall -Wextra -O2 -MMD -Werror -Wpedantic -g
DEBUG_FLAG=-DDEBUG_F
SERVER_FLAG=-DSERVER_F
# LDFLAGS=

SRCS=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.d,$(SRCS))
DEBUG_OBJS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%-debug.o,$(SRCS))

# remove the main.o of the main program
TEST_OBJS=$(subst $(BUILD_DIR)/main.o,,$(OBJS))

DOCS=$(wildcard $(DOC_DIR)/*.typ)

$(PROJ): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

debug: $(DEBUG_OBJS)
	$(CC) $(DEBUG_FLAG) $(CFLAGS) -o $(PROJ) $^


$(SERVER): $(OBJS)
	$(CC) $(SERVER_FLAG) $(CFLAGS) -o $@ $^

server-debug: $(DEBUG_OBJS)
	$(CC) $(DEBUG_FLAG) $(CFLAGS) -o $(SERVER) $^


test: test/main.c $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/test $^ && \
	./$(BUILD_DIR)/test -v

doc: $(DOCS) doc/refs.bib doc/main_loop_state_machine.dot
	dot -Tpng doc/main_loop_state_machine.dot -o doc/main_loop_state_machine.png
	typst c doc/documentation.typ documentation.pdf

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%-debug.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(DEBUG_FLAG) $(CFLAGS) -c $< -o $@

-include $(DEPS)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(PROJ)
