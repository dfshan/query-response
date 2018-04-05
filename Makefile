CFLAGS := -Wall
OBJS := server.o client.o sock.o
INCLUDES := main.h sock.h
LIBS := -lpthread
TARGETS := client server
all: $(TARGETS)
client: client.o sock.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
server: server.o sock.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
client.o: $(INCLUDES)
server.o: $(INCLUDES)
sock.o: sock.c sock.h
.PHONY: clean
clean:
	$(RM) $(TARGETS) $(OBJS)
