# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g -O2

# Executable name
TARGET = hashtablescratch

# Object files
OBJECTS = hashtablescratch_main.o hashtablescratch.o

# Build rules
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Compile hashtablescratch.c into hashtablescratch.o
hashtable.o: hashtablescratch.c hashtablescratch.h
	$(CC) $(CFLAGS) -c hashtablescratch.c

# Compile hashtablescratch_main.c into hashtablescratch_main.o
hashtablescratch_main.o: hashtablescratch_main.c hashtablescratch.h
	$(CC) $(CFLAGS) -c hashtablescratch_main.c

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJECTS)

# Rule to compile with ASanitizer (memory debugging)
debug: CFLAGS += -fsanitize=address -fno-omit-frame-pointer
debug: $(OBJECTS)
	$(CC) $(OBJECTS) -fsanitize=address -o $(TARGET)_debug

# Rule to run the program
run: $(TARGET)
	./$(TARGET)

# Rule to run the program with ASanitizer
run_debug: debug
	./$(TARGET)_debug