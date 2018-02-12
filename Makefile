DESTDIR?=/usr
PREFIX?=/local

ifneq ($V,1)
Q ?= @
endif

CC	= gcc
CFLAGS	= $(DEBUG) -Wall -Wextra $(INCLUDE) -Winline -pipe 

LDFLAGS	= -L$(DESTDIR)$(PREFIX)/lib
LIBS    = -lwiringPi -lwiringPiDev -lpthread -lrt -lm -lcrypt

SRC	=	megaio.c comm.c

OBJ	=	$(SRC:.c=.o)

all:		megaio

megaio:	$(OBJ)
	$Q echo [Link]
	$Q $(CC) -o $@ $(OBJ) $(LDFLAGS) $(LIBS)

.c.o:
	$Q echo [Compile] $<
	$Q $(CC) -c $(CFLAGS) $< -o $@

.PHONY:	clean
clean:
	$Q echo "[Clean]"
	$Q rm -f $(OBJ) megaio *~ core tags *.bak

.PHONY:	install
install: megaio
	$Q echo "[Install]"
	$Q cp megaio		$(DESTDIR)$(PREFIX)/bin
ifneq ($(WIRINGPI_SUID),0)
	$Q chown root.root	$(DESTDIR)$(PREFIX)/bin/megaio
	$Q chmod 4755		$(DESTDIR)$(PREFIX)/bin/megaio
endif
#	$Q mkdir -p		$(DESTDIR)$(PREFIX)/man/man1
#	$Q cp megaio.1		$(DESTDIR)$(PREFIX)/man/man1

.PHONY:	uninstall
uninstall:
	$Q echo "[UnInstall]"
	$Q rm -f $(DESTDIR)$(PREFIX)/bin/megaio
	$Q rm -f $(DESTDIR)$(PREFIX)/man/man1/megaio.1
