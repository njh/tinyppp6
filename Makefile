CFLAGS=-g -Wall
LDFLAGS=


all: tinyppp6

tinyppp6: tinyppp6.o frame.o fcs.o lcp.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c -o $@ $<

clean:
	rm -f *.o tinyppp6
	
.PHONY: all clean
