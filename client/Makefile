CC = gcc
CFLAGS = -Wall -Wextra -g
OBJ_DIR = .
SRC_DIR = src
INCLUDE_DIR = header
TARGET = client
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
INC = -I$(INCLUDE_DIR)
LIBS = -lncurses -lpthread

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INC) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	$(RM) $(OBJ_DIR)/*.o $(TARGET)
