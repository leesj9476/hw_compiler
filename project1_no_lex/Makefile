#
# Makefile for TINY
# Gnu C Version
# K. Louden 2/3/98
#

CC = gcc

CFLAGS = 

OBJDIR = obj

OBJS = $(addprefix $(OBJDIR)/, main.o util.o scan.o)

all: $(OBJDIR) cminus

cminus: $(OBJS)
	$(CC) $(CFLAGS) $^ -o cminus

$(OBJDIR)/main.o: main.c globals.h util.h scan.h
	$(CC) $(CFLAGS) -c main.c -o $(OBJDIR)/main.o

$(OBJDIR)/util.o: util.c util.h globals.h
	$(CC) $(CFLAGS) -c util.c -o $(OBJDIR)/util.o

$(OBJDIR)/scan.o: scan.c scan.h util.h globals.h
	$(CC) $(CFLAGS) -c scan.c -o $(OBJDIR)/scan.o

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	-rm cminus
	-rm $(OBJS)
	-rm -r $(OBJDIR)

