CFLAGS += -std=gnu11 -gdwarf-2 -D VERBOSE
EXES   =  q2a_prefetch  q2b_aio  q2b_pthreads   q2a_cached  q2a_disk_latency q2b_bigread
LDLIBS   +=  -lpthread -lrt

all: $(EXES)

clean:
	rm -f $(EXES) 
