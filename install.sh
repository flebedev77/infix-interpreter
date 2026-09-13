#!/bin/sh

gcc -std=c99 -O3 -flto -march=native -Wall -Wextra -Wpedantic -o calc_release main.c -lm
sudo cp ./calc_release /usr/local/bin/calc
