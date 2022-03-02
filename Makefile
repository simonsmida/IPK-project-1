bin_name=hinfosvc

src = $(wildcard *.c)
obj = $(src: .c = .o)

# TODO gnu99
LDFLAGS=-lm 
CFLAGS=-std=gnu99 -Wall -Wextra -pedantic -g

$(bin_name): $(obj)
	$(CC) $(CFLAGS) -o $@ $^ #$(LDFLAGS)

.PHONY: clean

clean:
	rm -f *.o $(bin_name)
