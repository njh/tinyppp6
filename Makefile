CFLAGS=-g -Wall
LDFLAGS=


all: tinyppp6

tinyppp6: tinyppp6.o hdlc.o ipv6cp.o fcs.o lcp.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c -o $@ $<

clean:
	rm -f *.o tinyppp6
	make -C tests clean

test:
	make -C tests test

.PHONY: all clean test
