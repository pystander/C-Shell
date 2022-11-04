COMPILE_CMD=gcc -I.
csrc = $(wildcard libs/cmd/*.c) \
		$(wildcard libs/signal/*.c) \
		$(wildcard libs/utils/*.c) \
		$(wildcard src/*.c)
obj = $(csrc:.c=.o)

all: clean csh

csh: $(obj)
	$(COMPILE_CMD) -o bin/$@ $^

%.o: %.c
	$(COMPILE_CMD) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(obj)