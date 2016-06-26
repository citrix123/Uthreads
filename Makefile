CFLAGS += -std=gnu11 -g 
EXES   =  ping_pong bounded_buffer
OBJS   =  ping_pong.o bounded_buffer.o uthread.o

all: $(EXES)

ping_pong: ping_pong.o uthread.o
bounded_buffer: bounded_buffer.o uthread.o

clean:
	rm -f $(EXES) $(OBJS)
tidy:
	rm -f $(OBJS)
