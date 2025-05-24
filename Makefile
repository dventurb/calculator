CC = gcc
CFLAGS = $(shell pkg-config --cflags gtk4) -I./lib -Wall -Wextra -g
LIBS = $(shell pkg-config --libs gtk4) -lm

SRC = main.c lib/tinyexpr.c
OBJ = $(SRC:.c=.o)
TARGET = calculator

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean

