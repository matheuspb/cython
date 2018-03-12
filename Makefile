.PHONY: all clean

TARGET = cython
SOURCE_DIR = src
INCLUDE_DIR = include

CPPFLAGS = -std=c++11 -I$(INCLUDE_DIR) -Wall -Wextra -g
LDFLAGS = -lstdc++

SCANNER_L = $(SOURCE_DIR)/scanner.l
SCANNER_SRC = $(SOURCE_DIR)/scanner.cpp

SRC_FILES = $(SCANNER_SRC)
OBJ_FILES = $(SRC_FILES:.cpp=.o)

all: $(TARGET)

I_TARGET = $(SCANNER_SRC:.cpp=)

$(SCANNER_SRC): $(SCANNER_L)
	flex -o $@ $^

$(I_TARGET): $(OBJ_FILES)
$(TARGET): $(I_TARGET)
	cp $^ $@

clean:
	rm -f $(TARGET) $(I_TARGET) $(OBJ_FILES) $(SCANNER_SRC)
