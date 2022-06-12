CC = gcc
CFLAGS = -Wall
SRC = $(shell find ./ -name '*.c')
HEADER = $(shell find ./ -name '*.H')
OBJ = $(SRC:%.c=%.o)
DEPS = $(HEADER)

#DEPS = src/init.h src/server/server.h src/client/client.h src/init/clicat.h
#OBJ = src/server/server.o src/client/client.o src/server/client_handle.o src/main.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

go: $(OBJ)
	gcc $(CFLAGS) -o $@ $^