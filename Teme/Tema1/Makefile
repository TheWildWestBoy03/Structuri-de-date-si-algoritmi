# Define variables
CC := gcc              # You can change this to another compiler (e.g., g++)
SRC := main.c          # Replace with your source files
OBJ := $(SRC:.c=.o)    # Convert .c files to .o object files
EXEC := sfl            # Name of the final executable
CFLAGS := -Wall        # Compiler flags, adjust as needed

# Default rule to build the executable
build: $(EXEC)

# Rule to link the object files and create the executable
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to run the executable
run_sfl: build
	./$(EXEC)

# Clean up by removing the executable and object files
clean:
	rm -f $(EXEC) $(OBJ)

# Optional: Declare phony targets to avoid conflicts with files of the same name
.PHONY: build run_sfl clean
