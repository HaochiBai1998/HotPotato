CFLAGS=-std=gnu99 -ggdb3
PROGS=ringmaster player
all: $(PROGS)
ringmaster: ringmaster.o socketInterface.o
	gcc -o $@ $^
player: player.o socketInterface.o
	gcc -o $@ $^
%.o: %.c
	gcc $(CFLAGS) -c $<
ringmaster.o:socketInterface.h
player.o:socketInterface.h
clean:
	rm -f $(PROGS)
