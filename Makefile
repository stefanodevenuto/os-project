all:
	gcc -c semaphore.c
	gcc master.c semaphore.o -o master
	gcc player.c semaphore.o -o player
	gcc pawn.c semaphore.o -o pawn

clean:
	rm -f *.o $(TARGET) *~

run: $(TARGET)
	./$(TARGET)
