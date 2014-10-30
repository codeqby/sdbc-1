
LIBDIR=$(SDBCDIR)/lib
INCDIR=$(SDBCDIR)/include
CFLAGS= -w -fPIC -I$(INCDIR) -O3
CC=gcc

LIBOBJS= getdw.o  tree.o tree_del.o  multi_hash.o Binary_search.o bound.o

$(LIBDIR)/libscbase.a:$(LIBOBJS)
	ar -rv $(LIBDIR)/libscbase.a $?
getdw.o tree.o tree_del.o:$(INCDIR)/tree.h
multi_hash.o:$(INCDIR)/multi_hash.h
Binary_search.o bound.o:$(INCDIR)/Binary_search.h

ttree:ttree.o 
	$(CC) -ottree ttree.o -L$(LIBDIR) -lsc
tbound:tbound.o 
	$(CC) -otbound tbound.o -L$(LIBDIR) -lsc
