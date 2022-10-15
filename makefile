CC = gcc
CFLAGS = -I include
OBJECTS = cards.o main.o
LDFLAGS = -lm

test: $(OBJECTS)
	$(CC) $(OBJECTS) -o bin/test $(LDFLAGS)

hearts: cards.o hearts.o
	$(CC) cards.o hearts.o -o bin/hearts $(LDFLAGS)

cards.o: cards.c include/cards.h
	$(CC) $(CFLAGS) -c cards.c

hearts.o: hearts.c include/cards.h include/hearts.h
	$(CC) $(CFLAGS) -c hearts.c

main.o: main.c include/cards.h
	$(CC) $(CFLAGS) -c main.c

debug: debug/debug debug/hearts

debug/debug: debug/main.o debug/cards.o
	$(CC) debug/main.o debug/cards.o -o debug/debug $(LDFLAGS)

debug/hearts: debug/cards.o debug/hearts.o
	$(CC) debug/cards.o debug/hearts.o -o debug/hearts $(LDFLAGS)

debug/main.o: main.c include/cards.h
	$(CC) $(CFLAGS) -c -g main.c -o debug/main.o

debug/cards.o: cards.o include/cards.h
	$(CC) $(CFLAGS) -c -g cards.c -o debug/cards.o

debug/hearts.o: hearts.c include/cards.h include/hearts.h
	$(CC) $(CFLAGS) -c -g hearts.c -o debug/hearts.o
