hard: master.c semaphore.c enviroment.c
	gcc -D HARD -c semaphore.c
	gcc -D HARD -c enviroment.c
	gcc -D HARD master.c semaphore.o enviroment.o -o master
	gcc -D HARD player.c semaphore.o enviroment.o -o player

easy: master.c semaphore.c enviroment.c
	gcc -c semaphore.c
	gcc -c enviroment.c
	gcc master.c semaphore.o enviroment.o -o master
	gcc player.c semaphore.o enviroment.o -o player

all: easy

clean:
	rm -f *.o $(TARGET) *~

run: $(TARGET)
	./$(TARGET)
