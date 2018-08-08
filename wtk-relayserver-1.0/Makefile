CC = gcc
#CFLAGS = -Wall -O -g
CFLAGS = -Wall -O2
TARGET = wtk-relayserver
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SOURCES))
$(TARGET) : $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)
	chmod a+x $(TARGET)

install:
	cp -f wtk-relayserver /usr/local/bin/

clean:
	rm -rf *.o wtk-relayserver
