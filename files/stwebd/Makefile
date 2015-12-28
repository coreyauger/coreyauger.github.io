#Out Definitions
OUT_NAME = stwebd

CFILES = main.c thread.c sighandles.c server.c
OBJECTS = main.o thread.o sighandles.o server.o


CC = gcc



CFLAGS = -Wall
LDLIBS =  -lpthread

.c.o:
	@echo Building $(@F)
	$(CC) -c $(CFLAGS) -o $@ $<
	@echo



$(OUT_NAME): $(OBJECTS) Makefile
	@echo Building application...
	$(CC) $(CFLAGS) $(OBJECTS) $(LDLIBS) -o $(OUT_NAME)
	@echo

clean:
	rm -f *.o *~ 

