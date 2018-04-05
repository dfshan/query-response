CFLAGS := -Wall -O2
OBJS := server.o client.o sock.o
INCLUDES := main.h sock.h
LIBS :=
TARGETS := client server
all: $(TARGETS)
client: client.o sock.o
server: server.o sock.o
client.o: $(INCLUDES)
server.o: $(INCLUDES)
sock.o: sock.c sock.h
.PHONY: clean
clean:
	$(RM) $(TARGETS) $(OBJS)
