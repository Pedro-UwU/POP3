# Source files
SRC := logger.c 

# Object files
OBJ = $(SRC:%.c=$(OBJDIR)/%.o)
OBJ_TEST = $(SRC:%.c=$(OBJDIR)/%.o)

# Target executable
EXEC :=

# Build rule
all: $(OBJ)

$(OBJ): $(SRC)
	@echo -e ${BLUE}${BOLD}"==> Utils object: $<"${NOCOLOR}
	$(CC) $(CFLAGS) -c -o $@ $<

test:
	@echo "TOOD"

# Cleanup rule
clean:
	@echo -e ${YELLOW}${BOLD}"==> Utils"${NOCOLOR}
	rm -f $(OBJ)

.PHONY: all test clean
