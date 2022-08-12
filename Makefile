# `make TTY=slang' will build with slang support
# `make TTY=termios' or just `make' will build with termios support

ifndef TTY
  TTY	= termios
endif
ifeq "$(TTY)" "slang"
  LIBS	= -lslang
endif
CFLAGS_F = -O3 -fno-builtin -funsigned-char
CFLAGS_W = -Wno-sign-compare -Wpointer-arith -Wno-system-headers -Wshadow -Wbad-function-cast -Werror
#-Wunreachable-code
CFLAGS	= $(CFLAGS_F) -ansi -Wall $(CFLAGS_W)
#-march=$(shell uname -m)
OBJ	= tty_$(TTY).o bf.o errors.o
BINARY	= bf
CLEAN	= *~ *.bak *.o $(BINARY)
LDFLAGS	= -s

.SUFFIXES: .c .o

all: $(OBJ)
	$(LINKER) $(LDFLAGS) $(OBJ) -o "$(BINARY)" $(LIBS)

%o: %c Makefile
	$(CC) $(CFLAGS) -c -o $@	$<

tty_termios.c: tty.h bf.h
tty_slang.c: tty.h errors.h bf.h
bf.c: bf.h tty.h errors.h
errors.c: errors.h

clean:
	rm -f $(CLEAN)

install:
	install -d $(PREFIX)
	install $(BINARY) $(PREFIX)

deb:
	dpkg-buildpackage -rfakeroot

deb-clean:
	rm -rf debian/bf debian/files debian/*substvars
	$(MAKE) clean
