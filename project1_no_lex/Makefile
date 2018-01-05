CC=gcc

CFLAGS=-g

OBJDIR=obj

OBJS=$(addprefix $(OBJDIR)/, main.o util.o scan.o)

FILENAME=cminus

all: dummy $(OBJDIR) $(FILENAME)

$(FILENAME): $(OBJDIR) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(FILENAME)

$(OBJDIR)/main.o: main.c globals.h util.h scan.h
	$(CC) $(CFLAGS) -c main.c -o $(OBJDIR)/main.o

$(OBJDIR)/util.o: util.c util.h globals.h
	$(CC) $(CFLAGS) -c util.c -o $(OBJDIR)/util.o

$(OBJDIR)/scan.o: scan.c scan.h util.h globals.h
	$(CC) $(CFLAGS) -c scan.c -o $(OBJDIR)/scan.o

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	-rm -f cminus
	-rm -f $(OBJS)
	-rm -rf $(OBJDIR)

