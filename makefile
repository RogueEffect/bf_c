CC = gcc
FLAGS = -g -Wall -std=c99
FILES = bf.c
EXE = bf

all:
	cd src/ && $(CC) $(FLAGS) $(FILES) -o ../$(EXE)
