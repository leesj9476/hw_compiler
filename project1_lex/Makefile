#
# Makefile for TINY
# Gnu C Version
# K. Louden 2/3/98
#

CC = gcc

CFLAGS = 

OBJDIR = obj

OBJS_FLEX = $(addprefix $(OBJDIR)/, main.o util.o lex.yy.o)

all: $(OBJDIR) cminus_flex

cminus_flex: $(OBJS_FLEX)
	$(CC) $(CFLAGS) $^ -o cminus_flex -lfl

$(OBJDIR)/main.o: main.c globals.h util.h scan.h
	$(CC) $(CFLAGS) -c main.c -o $(OBJDIR)/main.o

$(OBJDIR)/util.o: util.c util.h globals.h
	$(CC) $(CFLAGS) -c util.c -o $(OBJDIR)/util.o

$(OBJDIR)/lex.yy.o: cminus.l util.h globals.h scan.h
	flex cminus.l
	$(CC) $(CFLAGS) -c lex.yy.c -o $(OBJDIR)/lex.yy.o -lfl

$(OBJDIR):
	mkdir $@

clean:
	-rm cminus_flex
	-rm $(OBJS_FLEX) lex.yy.c
	-rm -r $(OBJDIR)


