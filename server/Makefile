include ../Makefile.commons

# Source files
SERVER_SRC = main.c serverUtils.c
SHARED_SRC = ../shared/logger.c ../shared/util.c

# Object files
SERVER_OBJ = $(SERVER_SRC:.c=.o)
SHARED_OBJ = $(SHARED_SRC:.c=.o)

# Target executable
SERVER_EXEC = server

# Build rule
all: $(SERVER_EXEC)

$(SERVER_EXEC): $(SERVER_OBJ) $(SHARED_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

# Cleanup rule
clean:
	rm -f $(SERVER_OBJ) $(SHARED_OBJ) $(SERVER_EXEC)

