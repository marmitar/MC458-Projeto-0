# TOOLS
CC = g++
# OPTIONS
LEVEL ?= 3
DEBUG ?= -DNDEBUG
WARNINGS ?= -Wall -Werror -Wpedantic -Wunused-result -D_FORTIFY_SOURCE=2
CFLAGS ?= -std=gnu++98 $(WARNINGS) $(DEBUG) -O$(LEVEL)

# SOURCE
SRC := mobile.cpp
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
$(PROG): $(SRC:.cpp=.o)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@
