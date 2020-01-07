all:
	gcc -std=c89 -Wpedantic -g -Og -c semaphore.c
	gcc -std=c89 -Wpedantic -g -Og master.c semaphore.o -o master -lm
	gcc -std=c89 -Wpedantic -g -Og player.c semaphore.o -o player -lm
	gcc -std=c89 -Wpedantic -g -Og pawn.c semaphore.o -o pawn

clean:
	rm -f *.o $(TARGET) *~

run: $(TARGET)
	./$(TARGET)
