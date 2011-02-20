
TARGET = sysalarm
OBJS = util.o config.o parse.o \
	condition_disk.o \
	condition_tcp.o \
	sysalarm.o  
CFLAGS = -Wall -Wno-pointer-sign -ggdb
LFLAGS = 

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LFLAGS) 
 
.c.o:
	$(CC) $(CFLAGS) -c $<

clean: 
	-rm *.o
	
indent:
	-indent -kr -i8 -l90 -nbbo -sc *.c ; rm *~