
LIBDIR=$(SDBCDIR)/lib
INCDIR=$(SDBCDIR)/include
CFLAGS= -m64 -w -fPIC -I$(INCDIR) -O3
CC=cc

LIBOBJS= chkexec.o echo.o filelist.o rexec.o  PutEvent.o \
	get_srvname.o PPC_srv.o TPC_srv.o clikey.o set_showid.o Function.o \
		 tpool.o 
$(LIBDIR)/libsc.a:$(LIBOBJS)
	ar -rv $(LIBDIR)/libsc.a $?
$(LIBOBJS):$(INCDIR)/scsrv.h $(INCDIR)/sc.h

