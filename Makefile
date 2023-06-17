# Some pieces are our own implementation, other inspiration is from
# https://github.com/mzahnd/sistemas-operativos-tp1/blob/main/Makefile
# https://github.com/torvalds/linux/blob/master/Makefile
# https://stackoverflow.com/questions/24144440/color-highlighting-of-makefile-warnings-and-errors


MAKE	:= make
MKDIR	:= mkdir -p

# Files
this-makefile	:= $(lastword $(MAKEFILE_LIST))
abs_srctree	:= $(realpath $(dir $(this-makefile)))

ifneq ($(words $(subst :, ,$(abs_srctree))), 1)
	$(error source directory cannot contain spaces or colons)
endif


SRCDIR	:= $(abs_srctree)/src
BINDIR	:= $(abs_srctree)/bin
OBJDIR	:= $(abs_srctree)/obj
INCLUDE	:= $(abs_srctree)/include

TEST_OBJFILES	=  $(wildcard $(TEST_OBJDIR)/*.o)
TEST_BINDIR 	:= $(abs_srctree)/bin/test
TEST_OBJDIR	:= $(abs_srctree)/obj/test
TEST_SRCDIR 	:= $(abs_srctree)/test

# Compiler and flags
LDLIBS	:= 
CC	:= gcc
CFLAGS	:= -g -std=c11 -pedantic \
		-Wall -Wextra \
		-fsanitize=address -fdiagnostics-color=auto \
		-I$(INCLUDE) $(LDLIBS)



export MAKE CC CFLAGS LDLIBS
export SRCDIR BINDIR INCLUDE OBJDIR
export TEST_SRCDIR TEST_BINDIR TEST_OBJDIR TEST_OBJFILES

# Bash Colors
BOLD	:= '\033[1m'
BLUE	:= '\033[34m'
YELLOW	:= '\033[33m'
NOCOLOR	:= '\033[0m'

export BOLD BLUE YELLOW NOCOLOR

UTILS	:= $(SRCDIR)/utils
SERVER	:= $(SRCDIR)/server

all: utils server

force: clean all

utils: $(BINDIR) $(OBJDIR)
	@$(MAKE) -C $(UTILS) $(1)

server: utils | $(BINDIR) $(OBJDIR)
	@$(MAKE) -C $(SERVER) $(1)

test: server utils | $(TEST_BINDIR)
	@echo "TODO"
	# @$(MAKE) -C $(SERVER) $(1)

# Create directories
$(BINDIR) $(OBJDIR) $(TEST_BINDIR):
	@$(MKDIR) $@

clean:
	$(MAKE) -C $(UTILS) clean
	$(MAKE) -C $(SERVER) clean


.PHONY: all test clean force
