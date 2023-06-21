# Source files
SRC = $(wildcard *.c)

OBJDIR_CLIENT := $(OBJDIR)/client
OBJDIR_UTILS := $(OBJDIR)/utils

# Object files
OBJ = $(SRC:%.c=$(OBJDIR_CLIENT)/%.o)

# Target executable
EXEC = $(BINDIR)/client

# All object files
OBJFILES = $(wildcard $(OBJDIR_UTILS)/*.o) $(wildcard $(OBJDIR_CLIENT)/*.o)

# Build rule
all: $(EXEC)

$(EXEC): $(OBJ) | $(OBJDIR_CLIENT)
	@echo -e ${BLUE}${BOLD}"==> Client executable: $@"${NOCOLOR}
	$(CC) $(CFLAGS) $(OBJFILES) -o $@

$(OBJ): $(OBJDIR_CLIENT)/%.o: %.c | $(OBJDIR_CLIENT)
	@echo -e ${BLUE}${BOLD}"==> Client object: $<"${NOCOLOR}
	$(CC) $(CFLAGS) -c -o $@ $< 

test:
	@echo "TOOD"

# Create directories
$(OBJDIR_CLIENT):
	$(MKDIR) $@

# Cleanup rule
clean:
	@echo -e ${YELLOW}${BOLD}"==> Client"${NOCOLOR}
	rm -f $(OBJ) $(EXEC)

.PHONY: all test clean
