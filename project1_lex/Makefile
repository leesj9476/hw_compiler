CC=gcc

CFLAGS=-g

OBJDIR=obj

OBJS_FLEX=$(addprefix $(OBJDIR)/, main.o util.o lex.yy.o)

FILENAME=cminus_flex

all: dummy $(OBJDIR) $(FILENAME)

$(FILENAME): $(OBJDIR) $(OBJS_FLEX)
	$(CC) $(CFLAGS) $(OBJS_FLEX) -o $(FILENAME) -lfl

$(OBJDIR)/main.o: main.c globals.h util.h scan.h
	$(CC) $(CFLAGS) -c main.c -o $(OBJDIR)/main.o

$(OBJDIR)/util.o: util.c util.h globals.h
	$(CC) $(CFLAGS) -c util.c -o $(OBJDIR)/util.o

$(OBJDIR)/lex.yy.o: cminus.l util.h globals.h scan.h
	flex cminus.l
	$(CC) $(CFLAGS) -c lex.yy.c -o $(OBJDIR)/lex.yy.o -lfl

$(OBJDIR):
	mkdir $(OBJDIR)

dummy:
	make clean

clean:
	-rm -f $(FILENAME)
	-rm -f $(OBJS_FLEX) lex.yy.c
	-rm -rf $(OBJDIR)
