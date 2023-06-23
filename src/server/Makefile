# MIT License - 2023
# Copyright 2023 - Lopez Guzman, Zahnd
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the “Software”), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to do
# so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
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
