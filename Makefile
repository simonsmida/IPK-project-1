bin_name=hinfosvc

src = $(wildcard *.c)
obj = $(src: .c = .o)

LDFLAGS=-lm 
CFLAGS=-std=c11 -Wall -Wextra -pedantic -g

$(bin_name): $(obj)
	$(CC) -o $@ $^ #$(LDFLAGS)

.PHONY: clean

clean:
	rm -f *.o $(bin_name)
