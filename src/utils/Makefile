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
