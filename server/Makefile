# Test Build

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = server
SRC = src/final_main.c src/final_db.c src/final_api.c src/final_sys.c src/final_task.c src/final_http.c src/util.c src/final_server.c 


OBJ_DIR = build
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.c=.o)))

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lmysqlclient -L/usr/lib/x86_64-linux-gnu/libcrypto.so.3 -lcrypto -lpthread

$(OBJ_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ_DIR)/*.o

