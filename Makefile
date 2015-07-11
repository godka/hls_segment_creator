CC=gcc
CFLAGS=-Wall -O3
CSOURCES=$(wildcard *.c)
INSTDIR = /usr/local/bin
COBJECTS=$(CSOURCES:.c=.o)
TARGET=hls_segment_creator

.PHONY: all clean

all: .dc $(CSOURCES) $(TARGET)

.dc: $(CSOURCES)
	$(CC) $(CFLAGS) -MM $(CSOURCES) >.dc
-include .dc
$(TARGET): $(COBJECTS)
	$(CC) $(COBJECTS) -o $@
clean:
	rm $(COBJECTS) .dc
install:$(TARGET)
	cp $(TARGET) $(INSTDIR)

