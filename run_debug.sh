#!/bin/sh
gcc main.c -o main -g -lm -DDEBUG && gf2 ./main
