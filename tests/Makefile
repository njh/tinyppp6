CFLAGS += -Werror -Wall -Wextra -pedantic -g -O0
CFLAGS += -I../src/

# Work around for compiling with clang
ifeq ($(shell $(CC) -v 2>&1 | grep -c "clang version"), 1)
  CFLAGS += -Wno-gnu-zero-variadic-macro-arguments
endif

CHECK_CFLAGS = $(shell pkg-config --cflags check)
CHECK_LIBS = $(shell pkg-config --libs check)

TEST_SUITES=$(wildcard *.tc)
TEST_SOURCES=$(TEST_SUITES:%.tc=%.c)
TEST_COMMANDS=$(TEST_SUITES:%.tc=%.cmd)


test: check

check: $(TEST_COMMANDS)
	@$(foreach f,$^,./$(f) &&) echo "All tests passed."

%.o: %.c
	$(CC) $(CXXFLAGS) $(CFLAGS) $(CHECK_CFLAGS) -c -o $@ $<

%.c: %.tc
	checkmk $< > $@ || rm -f $@

%.cmd: %.o
	$(CC) -o $@ $< -L. $(CHECK_LIBS) $(CFLAGS)


clean:
	rm -f $(TEST_SOURCES) *.o *.cmd

.PHONY: test check clean
