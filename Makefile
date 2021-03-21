DEBUG=yes
LIBCMINI=/home/vincent/Atari/Crossdev/libcmini
TOOLCHAIN=m68k-atari-mint-
CC=$(TOOLCHAIN)gcc
AS=$(TOOLCHAIN)as
STRIP=$(TOOLCHAIN)strip
CFLAGS=-I$(LIBCMINI)/include -DLIBCMINI -fomit-frame-pointer -Os
LDFLAGS=-L$(LIBCMINI)/build -lcmini -lgcc -lgem

EXEC=notepad.acc

SRC=$(wildcard *.c *.s)
OBJ=$(SRC:.c=.o)

# all: $(EXEC)
# ifeq ($(DEBUG),yes)
#     @echo "Génération en mode debug"
# else
#     @echo "Génération en mode release"
# endif

$(EXEC): $(OBJ)
	$(CC) -o $@ -nostdlib $(LIBCMINI)/build/crt0.o $^ -s $(LDFLAGS)   
	$(STRIP) $@

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

%.o: %.s
	$(CC) -o $@ -c $< $(CFLAGS)


.PHONY: clean mrproper

clean:
	@rm -rf *.o *~

mrproper: clean
	@rm -rf $(EXEC)
