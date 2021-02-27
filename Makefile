build:
	make -C src

clean:
	make -C src clean
	make -C tests clean

test: build
	make -C tests test

.PHONY: build clean test
