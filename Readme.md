# Assignment 2 Branch

## Description
For this assignment you will write a parallel program that takes a list of file names as input and produces a new set of files, which contain the english alphabet histograms for each of the input files. 

Histograms are simply counts - in this case, counts of alphabet characters in the file. For example, given a file with contents
```
hello world
```
the characters a thorough d appear 0 times, e - 1 time, etc..

Your program will consist of several processes - a parent and multiple children. These processes will communicate using pipes and signals.

## Compilation and running instructions

**To compile:** make A2

**To run:** ./A2
