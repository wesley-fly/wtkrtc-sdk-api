CC = gcc
#CFLAGS = -Wall -O -g
CFLAGS = -Wall -O2
TARGET = wtkrtc_proxy_server
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SOURCES))
$(TARGET) : $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)
	chmod a+x $(TARGET)

install:
	cp -f wtkrtc_proxy_server /usr/local/bin/

clean:
	rm -rf *.o wtkrtc_proxy_server
