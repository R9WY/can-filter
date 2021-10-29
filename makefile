CC=g++
PREFIX = /usr/local/bin
INC=-I /usr/local/include -L /usr/local/lib
LIB=
CFLAGS=$(INC) $(LIB) -m32 -c 
LDFLAGS=$(INC) $(LIB)

SOURCES=main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=can-filter

.PHONY: all clean install uninstall backup


all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(EXECUTABLE) *.o

install:
	cp $(EXECUTABLE) $(PREFIX)
uninstall:
	rm -rf $(PREFIX)/$(EXECUTABLE)
backup:
	$ tar -cvzf  ../archive/${EXECUTABLE}-`date +%Y%m%d`.tar.gz *.cpp *.h lib makefile
