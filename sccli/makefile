
LIBDIR=$(SDBCDIR)/lib
INCDIR=$(SDBCDIR)/include
CFLAGS= -m64 -w -fPIC -I$(INCDIR) -O3
CC=cc

LIBOBJS= client.o getls.o sqlc.o sqlinit.o getsvc.o netconnect.o get_clikey.o

$(LIBDIR)/libsc.a:$(LIBOBJS)
	ar -rv $(LIBDIR)/libsc.a $?
$(LIBOBJS):$(INCDIR)/sc.h $(INCDIR)/strproc.h $(INCDIR)/strproc.h

