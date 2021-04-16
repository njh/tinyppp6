CFLAGS=-g -Wall
LDFLAGS=

build: tinyppp6

tinyppp6: tinyppp6.o hdlc.o ipv6.o ipv6cp.o fcs.o lcp.o serial.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c tinyppp.h
	$(CC) -c -o $@ $<

clean:
	rm -f *.o tinyppp6

.PHONY: build clean
