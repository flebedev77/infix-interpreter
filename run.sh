#!/bin/sh
gcc main.c -o main -g -lm 
if [ $? == "0" ]; then
  ./main
fi
