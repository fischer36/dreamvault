# Test Build

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = server
SRC = src/server_linux.c src/db_testing.c src/tasks_testing.c src/http_xd.c src/api_xd.c src/http_parser.c src/sys.c src/util.c
OBJ_DIR = build
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.c=.o)))

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lmysqlclient -lcrypto

$(OBJ_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ_DIR)/*.o

