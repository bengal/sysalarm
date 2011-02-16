
CC = colorgcc
TARGET = sysalarm
OBJS = util.o config.o alarm_disk.o alarm_tcp.o sysalarm.o  
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