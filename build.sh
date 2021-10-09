#!/bin/bash

g++ -c  main.cpp -lpthread
g++ -O3  -o covidProgram main.o -lpthread

