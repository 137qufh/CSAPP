# Shared compiler flags. Chapters and labs include this file.
CC      ?= gcc
CFLAGS  ?= -Wall -Wextra -std=gnu11 -g -O1 -fwrapv -D_GNU_SOURCE
LDFLAGS ?=

ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

# When included from common/, ROOT is common/. From chapters/chXX it is still
# the directory of this file because of abspath on MAKEFILE_LIST.
SC_INC := $(ROOT)/include
SC_LIB := $(ROOT)/libscraft.a
