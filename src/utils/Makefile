# Source files
SRC := $(wildcard *.c)

OBJDIR_UTILS := $(OBJDIR)/utils

# Object files
OBJ = $(SRC:%.c=$(OBJDIR_UTILS)/%.o)
OBJ_TEST = $(SRC:%.c=$(OBJDIR_UTILS)/%.o)

# Target executable
EXEC :=

# Build rule
all: $(OBJ)

$(OBJ): $(OBJDIR_UTILS)/%.o: %.c | $(OBJDIR_UTILS)
	@echo -e ${BLUE}${BOLD}"==> Utils object: $<"${NOCOLOR}
	$(CC) $(CFLAGS) -c -o $@ $<

test:
	@echo "TOOD"

# Create directories
$(OBJDIR_UTILS):
	$(MKDIR) $@

# Cleanup rule
clean:
	@echo -e ${YELLOW}${BOLD}"==> Utils"${NOCOLOR}
	rm -f $(OBJ)

.PHONY: all test clean
