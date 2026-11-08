# 598APE-HW2

## Tools

This program assumes the following are installed on your machine:
* gcc
* make

If any of the above is missing, you can run the following commands in a terminal:
* `sudo apt install build-essential`

## Instructions

After pulling the code, open a terminal in the folder the code was pulled into.\
Run `make`.

### Default program usage:
```
./sandpile.exe <N>
```
N defines the 2D grid size.

### Optimized program usage:
```
./sandpile.exe <N> <optimizations>
```
The optimizations argument should be a chain of optimization flags connected with a "+" character.\
Note that although each optimization is independent in theory, the program implements each one on top of previous ones, so all preceding optimizations should be included.

Examples:
```
./genetic_benchmark <test name> o1
```
```
./genetic_benchmark <test name> o1+o2+o3+o4
```

## Optimizations

Optimization flags correspond to the following:
* o1: precompute the bounds for the Pac-Man shape.
* o2: topple all of the toppleable grains at once.
* o3: parallelize with OpenMP.
* o4: mirror the stabilized top half to the bottom half.
