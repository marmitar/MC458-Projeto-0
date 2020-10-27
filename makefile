# TOOLS
CC ?= gcc
# OPTIONS
LEVEL ?= 3
DEBUG ?= -DNDEBUG
WARNINGS ?= -Wall -Werror -Wpedantic -Wunused-result -D_FORTIFY_SOURCE=2
CFLAGS ?= -std=gnu99 $(WARNINGS) $(DEBUG) -O$(LEVEL)

# SOURCE
SRC := mobile.c
# ARTIFACT
PROG := main
PROJ := 0

# GENERAL COMMANDS
.PHONY: all run clean debug zip

all: $(PROG)

run: $(PROG)
	./$(PROG)

zip: $(PROJ).zip

debug: LEVEL = 0
debug: DEBUG = -DDEBUG
debug: CFLAGS += -ggdb3
debug: clean all

clean:
	rm -rf *.o $(PROG) *.zip

# BUILDING
$(PROG): $(SRC:.c=.o)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
