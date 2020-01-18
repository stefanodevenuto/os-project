

OBJ = semaphore.o Makefile
ALL = master
MASTER = master
PLAYER = player
PAWN = pawn

FLAGS = -std=c89 -Wpedantic -g -Og

$(MASTER): $(OBJ) $(MASTER).c
	$(CC) $(FLAGS) $(MASTER).c semaphore.o -o $(MASTER) -lm

$(PLAYER): $(OBJ) $(PLAYER).c
	$(CC) $(FLAGS) $(PLAYER).c semaphore.o -o $(PLAYER) -lm

$(PAWN): $(OBJ) $(PAWN).c
	$(CC) $(FLAGS) $(PAWN).c semaphore.o -o $(PAWN) -lm
	

all: $(MASTER) $(PLAYER) $(PAWN)

clean:
	rm -f *.o $(MASTER) $(PLAYER) $(PAWN)

run: all
	./$(MASTER)
