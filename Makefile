all:
	gcc -g -Og -c semaphore.c
	gcc -g -Og master.c semaphore.o -o master
	gcc -g -Og player.c semaphore.o -o player
	gcc -g -Og pawn.c semaphore.o -o pawn

clean:
	rm -f *.o $(TARGET) *~

run: $(TARGET)
	./$(TARGET)
