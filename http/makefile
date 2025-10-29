CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -g

TARGET_CLIENT = client
SRCS_CLIENT = http_client.c
OBJS_CLIENT = $(SRCS_CLIENT:.c=.o)
LDFLAGS_CLIENT =

TARGET_SERVER = server
SRCS_SERVER = http_server.c
OBJS_SERVER = $(SRCS_SERVER:.c=.o)

LDFLAGS_SERVER = -lpthread

all: $(TARGET_CLIENT) $(TARGET_SERVER)
client: $(TARGET_CLIENT)
server: $(TARGET_SERVER)

$(TARGET_CLIENT): $(OBJS_CLIENT)
	$(CC) $(LDFLAGS_CLIENT) -o $@ $^

$(TARGET_SERVER): $(OBJS_SERVER)
	$(CC) $(LDFLAGS_SERVER) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET_CLIENT) $(OBJS_CLIENT) $(TARGET_SERVER) $(OBJS_SERVER)

.PHONY: all client server clean