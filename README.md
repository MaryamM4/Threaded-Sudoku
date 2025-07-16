# Sudoku Verifier and Solver

Validates and partially solves NxN Sudoku puzzles using a constraint-oriented design.

Instead of per-cell candidate tracking, the system models state through house structures (row, column, box) to enable direct consistency checks and constraint propagation. Each house tracks candidate availability via fixed-size integer arrays. Candidate discovery uses locked candidate logic derived from the intersection of the three relevant houses (row ∩ column ∩ box).

The architecture was designed to support future binary representations and bitwise operations for faster constraint resolution, though this is not currently implemented. Solving strategy is limited to naked singles and does not support recursion or backtracking.

**Note:**  
Multithreaded consistency checks via pthreads are included for completeness but do not yield performance gains yet.


## Results
For puzzles that have any "0"s, tries to find a valid number for the 0. Can solve simple puzzles where no backtracking is required.

2x2 puzzle

```
3 0 | 0 1
2 1 | 0 0
---------
0 0 | 0 2
4 2 | 1 0
```
Finds grid[1][2] as 4, grid[4][4] as 3
```
3 4 | 0 1
2 1 | 0 0
---------
0 0 | 0 2
4 2 | 1 3
```
Finds grid[1][3] ad 2, grid[3][1] as 1, grid[3][2] as 3
```
3 4 | 2 1
2 1 | 0 0
---------
1 3 | 0 2
4 2 | 1 3
```
Finds grid[2][4] as 4, grid[3][3] ad 4
```
3 4 | 2 1
2 1 | 0 4
---------
1 3 | 4 2
4 2 | 1 3
```
Finds grid[2][3] as 3
```
3 4 | 2 1
2 1 | 3 4
---------
1 3 | 4 2
4 2 | 1 3
```

The current version would not be able to solve a more complex puzzle, such as 
```
3 0 | 0 0
2 1 | 0 0
---------
0 0 | 0 0
4 2 | 1 0
```
