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

ENVDIR	:= $(abs_srctree)/env
SRCDIR	:= $(abs_srctree)/src
BINDIR	:= $(abs_srctree)/bin
OBJDIR	:= $(abs_srctree)/obj
INCLUDE	:= $(abs_srctree)/include

TEST_OBJFILES	=  $(wildcard $(TEST_OBJDIR)/*.o)
TEST_BINDIR 	:= $(abs_srctree)/bin/test
TEST_OBJDIR	:= $(abs_srctree)/obj/test
TEST_SRCDIR 	:= $(abs_srctree)/test

MAILDROPDIR	:= $(abs_srctree)/bin/maildirs

# Compiler and flags
LDLIBS	:= 
CC	:= gcc
CFLAGS	:= -g -std=c11 -Wno-variadic-macros -Wno-unused-parameter \
		-Wall -Wextra -Wno-stringop-truncation \
		-fsanitize=address -fdiagnostics-color=auto \
		-I$(INCLUDE) $(LDLIBS)

export MAKE MKDIR
export CC CFLAGS LDLIBS
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
CLIENT	:= $(SRCDIR)/client

all: utils server client

force: clean all

utils: $(BINDIR) $(OBJDIR)
	@$(MAKE) -C $(UTILS) $(1)

server: utils | $(BINDIR) $(OBJDIR) $(MAILDROPDIR)
	@$(MAKE) -C $(SERVER) $(1)

client: utils | $(BINDIR) $(OBJDIR)
	@$(MAKE) -C $(CLIENT) $(1)

test: server utils | $(TEST_BINDIR)
	@echo "TODO"
	# @$(MAKE) -C $(SERVER) $(1)

# Create directories
$(BINDIR) $(OBJDIR) $(TEST_BINDIR) $(MAILDROPDIR):
	@$(MKDIR) $@

clean:
	$(MAKE) -C $(UTILS) clean
	$(MAKE) -C $(SERVER) clean
	$(MAKE) -C $(CLIENT) clean

format:
	@$(ENVDIR)/restyle.sh

.PHONY: all force utils server client test clean format
