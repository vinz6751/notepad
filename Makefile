DEBUG=yes
CC=m68k-atari-mint-gcc
CFLAGS=-fomit-frame-pointer -Os
LDFLAGS=-lgem
EXEC=notepad.acc
SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)

# all: $(EXEC)
# ifeq ($(DEBUG),yes)
#     @echo "Génération en mode debug"
# else
#     @echo "Génération en mode release"
# endif

notepad.acc: $(OBJ)
	@$(CC) -o $@ $^ $(LDFLAGS)

#notepad.c: hello.h

%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	@rm -rf *.o *~

mrproper: clean
	@rm -rf $(EXEC)
