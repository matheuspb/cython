.PHONY: all format clean

TARGET = cython
SOURCE_DIR = src
INCLUDE_DIR = include

CPPFLAGS = -std=c++11 -I$(INCLUDE_DIR) -Wall -Wextra -g
LDFLAGS = -lstdc++

SCANNER_L = $(SOURCE_DIR)/scanner.l
SCANNER_SRC = $(SOURCE_DIR)/scanner.cpp

PARSER_Y = $(SOURCE_DIR)/parser.y
PARSER_H = $(INCLUDE_DIR)/parser.h
PARSER_SRC = $(SOURCE_DIR)/parser.cpp

SRC_FILES = $(SCANNER_SRC) $(PARSER_SRC) $(SOURCE_DIR)/ast.cpp
OBJ_FILES = $(SRC_FILES:.cpp=.o)

I_TARGET = $(PARSER_SRC:.cpp=)

all: $(TARGET)

$(SCANNER_SRC): $(SCANNER_L)
	flex -o $@ $^

$(PARSER_SRC) $(PARSER_H): $(PARSER_Y)
	bison -o $(PARSER_SRC) --defines=$(PARSER_H) $^
	mv $(SOURCE_DIR)/*.hh $(INCLUDE_DIR)

$(I_TARGET): $(OBJ_FILES)
$(TARGET): $(I_TARGET)
	cp $^ $@

format:
	clang-format --style=file -i **/*.{h,cpp}

clean:
	rm -f $(TARGET) $(I_TARGET) $(OBJ_FILES)
	rm -f $(SCANNER_SRC) $(PARSER_SRC) $(PARSER_H) $(INCLUDE_DIR)/*.hh
