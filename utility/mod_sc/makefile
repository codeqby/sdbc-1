# mid_status 支持有状态连接的交易管理器  
CFLAGS = -m64 -w -O3 -I$(SDBCDIR)/include
LFLAGS = -L$(SDBCDIR)/lib 

OBJS = login.o transfer.o mod_sc.o scpool.o
midsc: midsc.o $(OBJS)
	$(CC) -m64 -o midsc midsc.o $(OBJS) $(LFLAGS) -lsc -lscbase -ljson -lpthread -lnsl -lrt
#-L/usr/local/lib -ltcmalloc_minimal

t:t.o
	cc -o t t.o
