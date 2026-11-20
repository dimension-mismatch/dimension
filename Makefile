CC := clang
INCLUDE := include

CFLAGS := -g -fsanitize=address -Wall -I./$(INCLUDE) -I./$(INCLUDE)/standalone
SRC := src
BUILD := build


HEADERS := $(shell find $(INCLUDE) -name '*.h' -not -path "*/standalone/*")
OBJECTS := $(patsubst $(INCLUDE)/%.h, $(BUILD)/%.o, $(HEADERS))

.PHONY: clean init all

all : init dmsn

dmsn : $(SRC)/dmsn.c $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

init :
	mkdir -p build
	rsync -a --include='*/' --exclude='*' --exclude='/standalone' ./$(SRC)/ ./$(BUILD)/


$(BUILD)/%.o : $(SRC)/%.c $(INCLUDE)/%.h
	$(CC) $(CFLAGS) -c -o $@ $<


clean :
	rm -f *.o
	rm -f validator/*.o
	rm -f hash_table/*.o
	rm -f error_handling/*.o
	rm -f dimension-IR/*.o
	rm -rf dmsn.dSYM