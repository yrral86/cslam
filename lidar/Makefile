#=================================================
# Makefile for URG sample programs (scip-samples)
#-------------------------------------------------
# Copyright (C) 2007 aokaze (Hara Yoshitaka)
# bluewind@roboken.esys.tsukuba.ac.jp
#=================================================

# User program source file 
# Add new source (program developed by user) below to compile.
SRC     = lidar.c ../share.c

# User program object file
OBJ     = ${SRC:.c=.o}

# Executable file
TARGET  = lidar

# Header file
HEAD    = scipBase.h scip1.h scip2.h scipUtil.h share.h

# API File
API_SRC = scipBase.c scip1.c scip2.c scipUtil.c share.c
API_OBJ = ${API_SRC:.c=.o}

CC      = gcc

INC_DIR = -I/usr/local/include

LIB_DIR = -L/usr/local/lib
LIBS    = -lm -lrt -pthread

# Debug build
CFLAGS  = -g -Wall -Wmissing-prototypes $(INC_DIR)

# Release build
#CFLAGS = -O2 -Wall -Werror -Wmissing-prototypes $(INC_DIR)

all: $(TARGET)

# Compile
$(API_OBJ) $(OBJ): $(HEAD)
.c.o:
	$(CC) $(CFLAGS) -c $<

# Link
$(TARGET): $(API_OBJ) $(OBJ)
	$(CC) $(LIB_DIR) $(LIBS) $(API_OBJ) $@.o -o $@

# Clean
clean:
	rm -f $(TARGET) *.o core*
