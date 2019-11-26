# dal carattere '#' fino a fine riga, il testo dentro il Makefile e`
# un commento
#
# flags per la compilazione
CFLAGS = -std=c89 -Wpedantic
# target ovvero nome dell'eseguibile che si intende produrre
TARGET = master
# object files necessari per produrre l'eseguibile
#OBJ    = test-list.o
DEPS   = master.c

# Si sfrutta la regola implicita per la produzione dei file oggetto in
# $(OBJ)
#
# Le "variabili" del Makefile sono espanse con $(NOME_VAR). Quindi
# scrivere
#
# $(TARGET): $(OBJ)
#
# equivale a scrivere
#
# application: matrix.o application.o
#
# ovvero a specificare che per produrre l'eseguibile "application"
# servono i due object files "matrix.o" e "application.o"

hard:  $(DEPS)
	$(CC) -D HARD $(DEPS) $(LDFLAGS) -o $(TARGET)

easy:  $(DEPS)
	$(CC) $(DEPS) $(LDFLAGS) -o $(TARGET)

# solitamente il target "all" e` presente

all: easy

# con "make clean" solitamente si vogliono eliminare i prodotti della
# compilazione e ripristinare il contenuto originale
clean:
	rm -f *.o $(TARGET) *~

# il target run si usa talvolta per eseguire l'applicazione
run: $(TARGET)
	./$(TARGET)
