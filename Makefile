
#############################################################################
# Parameters
COMPILER ?= gnu
MEMORY ?= dynamic
KERNEL ?= openmp
NTIMES ?= 20
STREAM_ARRAY_SIZE ?= 2147483648

# Suggested sizes for Fujitsu A64FX
# Total memory: 24GB
#STREAM_ARRAY_SIZE ?= 1073741824
# Total memory: 6GB (one CMG)
#STREAM_ARRAY_SIZE ?= 268435440
# A convinent stream size: (48[cores] * 32[elem per cache line] * 400000)
#STREAM_ARRAY_SIZE ?= 614400000
#############################################################################


#############################################################################
# Compiler
ifeq (gnu,$(COMPILER))
CC = gcc
CFLAGS = -Ofast -mcpu=native -fopenmp
LD = $(CC)
LDFLAGS = $(CFLAGS)
#HUGETLBFS = LD_PRELOAD=libhugetlbfs.so HUGETLB_MORECORE=2M HUGETLB_VERBOSE=99
OMP_ENV = OMP_NUM_THREADS=64 OMP_PROC_BIND=close
#NUMACTL = numactl -C 0-80 -l
RUN = $(HUGETLBFS) $(OMP_ENV) $(NUMACTL)

else
#----------------------------------------------------------------------------
ifeq (acfl,$(COMPILER))
CC = armclang
CFLAGS = -Ofast -mcpu=native -fopenmp
LD = $(CC)
LDFLAGS = $(CFLAGS)
#HUGETLBFS = LD_PRELOAD=libhugetlbfs.so HUGETLB_MORECORE=2M HUGETLB_VERBOSE=99
OMP_ENV = OMP_NUM_THREADS=64 OMP_PROC_BIND=close
#NUMACTL = numactl -C 0-80 -l
RUN = $(HUGETLBFS) $(OMP_ENV) $(NUMACTL)

else
#----------------------------------------------------------------------------
ifeq (fujitsu,$(COMPILER))
CC = fcc
CFLAGS = -Kfast,preex -Kopenmp -Kzfill -Kstriping=4
LD = $(CC)
LDFLAGS = $(CFLAGS)
OMP_ENV = OMP_NUM_THREADS=48
#NUMACTL = numactl -C 0-47
FJ_ENV = \
    FLIB_SCCR_CNTL=TRUE \
    FLIB_HPCFUNC=TRUE \
    FLIB_HPCFUNC_INFO=TRUE \
    FLIB_FASTOMP=TRUE \
    FLIB_BARRIER=HARD \
    XOS_MMM_L_PRINT_ENV=on \
    XOS_MMM_L_PAGING_POLICY=demand:demand:demand \
    XOS_MMM_L_ARENA_LOCK_TYPE=1
RUN = $(FJ_ENV) $(OMP_ENV) $(NUMACTL)

else
#----------------------------------------------------------------------------
$(error Invalid parameter: COMPILER=$(COMPILER))
endif
endif
endif
#############################################################################


#############################################################################
# Machinery -- only edit if you know what you're doing
ifeq (dynamic,$(MEMORY))
STREAM_ARRAY_DYNAMIC = 1
else
ifeq (static,$(MEMORY))
CFLAGS += -mcmodel=large
STREAM_ARRAY_DYNAMIC = 0
else
$(error Invalid parameter: MEMORY=$(MEMORY))
endif
endif

CFLAGS += -DSTREAM_ARRAY_DYNAMIC=$(STREAM_ARRAY_DYNAMIC) -DSTREAM_ARRAY_SIZE=$(STREAM_ARRAY_SIZE) -DNTIMES=$(NTIMES)

ifeq (,$(KERNEL))
EXE = stream.exe
OBJS = stream.o
else
CFLAGS += -DSTREAM_KERNEL=$(KERNEL)
EXE = stream_$(KERNEL).exe
OBJS = stream.o kernel-$(KERNEL).o
endif

EXES = $(EXE)
HDRS = $(wildcard *.h)

.PHONY: all clean run

all: $(EXES)

clean:
	rm -f *.o *.exe

run: $(EXE)
	$(RUN) ./$^

$(EXE): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c Makefile $(HDRS)
	$(CC) -c $(CFLAGS) -o $@ $<

#############################################################################