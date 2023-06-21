# Source files
SRC = $(wildcard *.c)

OBJDIR_SERVER := $(OBJDIR)/server
OBJDIR_UTILS := $(OBJDIR)/utils

# Object files
OBJ = $(SRC:%.c=$(OBJDIR_SERVER)/%.o)

# Target executable
EXEC = $(BINDIR)/server

# All object files
OBJFILES = $(wildcard $(OBJDIR_UTILS)/*.o) $(wildcard $(OBJDIR_SERVER)/*.o)

# Build rule
all: $(EXEC)

$(EXEC): $(OBJ) | $(OBJDIR_SERVER)
	@echo -e ${BLUE}${BOLD}"==> Server executable: $@"${NOCOLOR}
	$(CC) $(CFLAGS) $(OBJFILES) -o $@

$(OBJ): $(OBJDIR_SERVER)/%.o: %.c | $(OBJDIR_SERVER)
	@echo -e ${BLUE}${BOLD}"==> Server object: $<"${NOCOLOR}
	$(CC) $(CFLAGS) -c -o $@ $< 

test:
	@echo "TOOD"

# Create directories
$(OBJDIR_SERVER):
	$(MKDIR) $@

# Cleanup rule
clean:
	@echo -e ${YELLOW}${BOLD}"==> Server"${NOCOLOR}
	rm -f $(OBJ) $(EXEC)

.PHONY: all test clean
