bin_name=hinfosvc
login=xsmida03

src = $(wildcard *.c)
obj = $(src: .c = .o)

CC = gcc
CFLAGS=-std=gnu99 -Wall -Wextra -pedantic -g

$(bin_name): $(obj)
	$(CC) $(CFLAGS) -o $@ $^ 

.PHONY: clean zip

zip:
	zip $(login).zip Makefile Readme.md *.c

clean:
	rm -f *.o $(bin_name)
