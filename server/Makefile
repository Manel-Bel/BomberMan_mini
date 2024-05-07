CC = gcc
CFLAGS = -Wall -Wextra -g 
SRC_DIR = src
OBJ_DIR = .
INCLUDE_DIR = header
TARGET = server
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
INC = -I$(INCLUDE_DIR)
LIBS = -lncurses -pthread -lrt

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INC) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	$(RM) $(OBJ_DIR)/*.o $(TARGET)
