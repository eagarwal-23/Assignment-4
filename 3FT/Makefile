#----------------------------------------------------------------------
# Makefile for Assignment 4, Part 3
# Author: Eesha Agarwal
#----------------------------------------------------------------------

# Macros
CC = gcc217
# CC = gcc217m
CFLAGS = -g
# CFLAGS = -D NDEBUG
# CFLAGS = -D NDEBUG -O

all: ft
clean: rm -f ft *~

ft: dynarray.o DTNode.o FileNode.o ft.o ft_client.c
	$(CC) $(CFLAGS) dynarray.o DTNode.o FileNode.o ft.o ft_client.c -o ft

dynarray.o: dynarray.c dynarray.h
	$(CC) $(CFLAGS) -c dynarray.c

DTNode.o: DTNode.c DTNode.h
	$(CC) $(CFLAGS) -c DTNode.c

FileNode.o: FileNode.c FileNode.h
	$(CC) $(CFLAGS) -c FileNode.c

ft.o: ft.c ft.h
	$(CC) $(CFLAGS) -c ft.c
