OBJS 	= reader.o record.o writer.o shm_linker.o shm_unlinker.o
SOURCE	= reader.c record.c writer.c shm_linker.c shm_unlinker.c
HEADER  = record.h memory.h
OUT  	= linker reader writer unlinker
CC	= gcc
FLAGS   = -Wall -lrt -g -c 

all: $(OUT)

unlinker: shm_unlinker.o 
		$(CC) -g shm_unlinker.o -o unlinker

shm_unlinker.o: 
		$(CC) $(FLAGS) shm_unlinker.c

linker: shm_linker.o 
		$(CC) -g shm_linker.o -o linker

shm_linker.o: 
		$(CC) $(FLAGS) shm_linker.c

reader: reader.o record.o
		$(CC) -g reader.o record.o -o reader

reader.o: reader.c
		$(CC) $(FLAGS) reader.c

writer: writer.o record.o
		$(CC) -g writer.o record.o -o writer

writer.o: writer.c
		$(CC) $(FLAGS) writer.c

record.o: record.c
		$(CC) $(FLAGS) record.c

clean:
	rm -f $(OBJS) $(OUT)

count: 
	wc $(SOURCE) $(HEADER)