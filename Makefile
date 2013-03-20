CC = gcc
CFLAGS = -std=c99
SRCS = srllist.c srllist.h

countTour: $(SRCS)
	$(CC) $(CFLAGS) -o srllist $(SRCS)

debug: $(SRCS)
	$(CC) $(CFLAGS) -g -ggdb -o srllist $(SRCS)

valgrind: $(SRCS)
	$(CC) $(CFLAGS) -ggdb -o srllist $(SRCS)
clean:
	rm -f *~
	rm -f ./srllist
