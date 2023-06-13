# Source files
SRC = $(wildcard *.c)


# Object files
OBJ = $(SRC:%.c=$(OBJDIR)/%.o)

# Target executable
EXEC = $(BINDIR)/server

# Build rule
all: $(EXEC)

$(EXEC): $(OBJ)
	@echo -e ${BLUE}${BOLD}"==> Server executable: $@"${NOCOLOR}
	$(CC) $(CFLAGS) $^ $(OBJFILES) -o $@

$(OBJ): $(OBJDIR)/%.o: %.c
	@echo -e ${BLUE}${BOLD}"==> Server object: $<"${NOCOLOR}
	$(CC) $(CFLAGS) -c -o $@ $< 

test:
	@echo "TOOD"

# Cleanup rule
clean:
	@echo -e ${YELLOW}${BOLD}"==> Server"${NOCOLOR}
	rm -f $(OBJ) $(EXEC)

.PHONY: all test clean
