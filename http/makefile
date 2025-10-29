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

# Regra 'all' é a padrão
all: $(TARGET_CLIENT) $(TARGET_SERVER)

# REGRA DE CONSTRUÇÃO DO CLIENTE
$(TARGET_CLIENT): $(OBJS_CLIENT)
	$(CC) $(LDFLAGS_CLIENT) -o $@ $^

# REGRA DE CONSTRUÇÃO DO SERVIDOR
$(TARGET_SERVER): $(OBJS_SERVER)
	$(CC) $(LDFLAGS_SERVER) -o $@ $^

# Regra genérica para .c -> .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra de limpeza
clean:
	rm -f $(TARGET_CLIENT) $(OBJS_CLIENT) $(TARGET_SERVER) $(OBJS_SERVER)

# 'all' e 'clean' são "falsos" (não são arquivos)
.PHONY: all clean