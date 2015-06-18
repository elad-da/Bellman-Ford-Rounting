
# compiler
CC=gcc

# compile arguments
CFLAGS+=-c -g -Wall -Wvla

# linker flags
LDFLAGS+=

# libraries
LIBS+=-lpthread

# our source files
SOURCES= BlRouting.c main.c

# a macro to define the objects from sources
OBJECTS=$(SOURCES:.c=.o)

# executable name
EXECUTABLE=bf

$(EXECUTABLE): $(OBJECTS)
	@echo "Building target" $@ "..."
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)
	@echo "Done!"
	
# a rule for generating object files given their c files
.c.o:
	@echo "Compiling" $< "..."
	$(CC) $(CFLAGS) $< -o $@
	@echo "Done!"
	
clean:
	@echo "Ceaning up *.o Files..."
	rm -rf *s *o 
	@echo "Done!"

.PHONY: all clean
