IDIR =./include
CC=gcc
CXX=g++
COMMONFLAGS = -lrt -O0

ODIR=./obj
LDIR=./lib
SDIR=./src
LIBS=


ifdef DEBUG
	COMMONFLAGS := $(COMMONFLAGS) -g
endif


ifeq ($(P4080), true)
	_EPU = photonEndTiming.o photonStartTiming.o photonReportTiming.o photonPrintTiming.o
	_DEFINE = -D__P4080
else
endif

BINFILE1 = producer
BINFILE2 = consumer

all: A B 

A: $(BINFILE1)
B: $(BINFILE2)

_DEPS = define.h timingUtils.h 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ1 = producer.o util.o $(_EPU)
OBJ1 = $(patsubst %,$(ODIR)/%,$(_OBJ1))

_OBJ2 = consumer.o util.o $(_EPU)
OBJ2 = $(patsubst %,$(ODIR)/%,$(_OBJ2))

    
$(ODIR)/%.o : $(SDIR)/%.c $(DEPS)
	$(CC) $(COMMONFLAGS) $(_DEFINE) -c $< -o $@ 

$(BINFILE1): $(OBJ1) 
	$(CC) -o $@ $^ $(COMMONFLAGS) $(_DEFINE) $(LIBS)

$(BINFILE2): $(OBJ2) 
	$(CC) -o $@ $^ $(COMMONFLAGS) $(_DEFINE) $(LIBS)


.PHONY: clean all

clean:
	rm -f $(ODIR)/*.o *~ $(BINFILE1) $(BINFILE2)
