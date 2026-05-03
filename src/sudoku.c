// Sudoku puzzle verifier and solver

#include "grid.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ORDER;

// takes filename and pointer to Grid of Houses
// returns grid values of Sudoku puzzle and fills houses
int **readSudokuPuzzle(char *filename, Grid *grid) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  if (strstr(filename, ".txt") == NULL) {
    fprintf(stderr, "Invalid file: File %s must be a text file.\n", filename);
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  int psize;
  if (fscanf(fp, "%d", &psize) != 1) {
    fprintf(stderr, "Failed to read puzzle size.\n");
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  // Check that number is perfect square
  int root = (int)sqrt(psize);
  if (psize <= 0 || (root * root) != psize) {
    fprintf(stderr, "Invalid puzzle size %d: Puzzle must be a perfect square.\n", psize);
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  ORDER = psize;
  grid->order = ORDER;

  for (int i = 0; i < NUM_HOUSE_TYPES; i++) {
    grid->houses[i] = initEmptyHouses();
  }

  int **grid_vals = malloc(psize * sizeof(int *));

  int val, box_idx;
  for (int row = 1; row <= psize; row++) {
    grid_vals[row - 1] = malloc(psize * sizeof(int));

    for (int col = 1; col <= psize; col++) {
      fscanf(fp, "%d", &val);
      grid_vals[row - 1][col - 1] = val;

      if (val == 0) {
        grid->complete = false;

      } else {
        box_idx = getBoxIdx(row - 1, col - 1, psize);

        addValue(grid->houses[box_t_idx][box_idx], val);
        addValue(grid->houses[row_t_idx][row - 1], val);
        addValue(grid->houses[col_t_idx][col - 1], val);
      }
    }
  }
  fclose(fp);
  return grid_vals;
}

void printSudokuPuzzle(int **grid_vals, int order) {
  printf("\n%d\n", order);
  for (int row = 0; row < order; row++) {
    for (int col = 0; col < order; col++) {
      printf("%d ", grid_vals[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

// takes puzzle size and grid[][]
// frees the memory allocated
void deleteSudokuPuzzle(Grid *grid, int **grid_vals) {
  for (int i = 0; i < grid->order; i++) {
    free(grid_vals[i]);
    grid_vals[i] = NULL;
  }
  free(grid_vals);
  grid_vals = NULL;

  delGrid(grid);
}

// Don't need to worry about threads writing at the same time,
// because they can only change bool to false,
// and if even 1 house returns is invalid/incomplete,
// the entire grid in invalid/incomplete.
void *checkHousesStatus(void *arg) {
  GridIterator *grid_it = (GridIterator *)arg;
  Grid *grid = grid_it->grid;
  int hs_t_idx = grid_it->house_type_idx;

  if (!grid->valid) {
    free(grid_it);
    return NULL;
  } // Grid alrdy known to be invalid from other threads.

  // Don't need to lock for reading
  for (int hs_idx = 0; hs_idx < grid->order; hs_idx++) {
    if (grid->houses[hs_t_idx][hs_idx][0] == INVALID && grid->valid) {
      grid->valid = false;

      printf("Invalid puzzle: A house should not have duplicate values.\n");
      // printf("Found duplicate: hs_t_idx=%d, hs_idx=%d.\n", hs_t_idx, hs_idx);
      //  printf("House counts: ");
      //  printCounts(grid->houses[hs_t_idx][hs_idx]);
      //  printf("\n");

    } else if (grid->houses[hs_t_idx][hs_idx][0] != FULL && grid->complete) {
      grid->complete = false;
    }
  }

  free(grid_it);
  return NULL;
}

void checkHouseType(Grid *grid, int hs_t_idx) {
  for (int hs_idx = 0; hs_idx < grid->order; hs_idx++) {
    if (grid->houses[hs_t_idx][hs_idx][0] == INVALID) {
      grid->valid = false;
      grid->complete = false;
      printf("Invalid puzzle: A house should not have duplicate values.\n");
      return;
    
    } else if (grid->houses[hs_t_idx][hs_idx][0] != FULL && grid->complete) {
      grid->complete = false;
    }
  }
}

void checkGridStatus(Grid *grid) {
  for (int t_idx = 0; t_idx < NUM_HOUSE_TYPES; t_idx++) {
    if (grid->houses[t_idx] == NULL) {
      printf("Grid has a NULL Houses. Invalidating.\n");
      grid->valid = false;
      grid->complete = false;
      return;
    }
  }

  // Assume innocent until proven guilty.
  grid->complete = true;
  grid->valid = true;

  for (int t_idx = 0; t_idx < NUM_HOUSE_TYPES; t_idx++) {
    checkHouseType(grid, t_idx);
  }
}

// Hidden single: A house has multiple candidates, but
//                only 1 cell can house a certain candidate.
// Any house type can have hidden single/pair/triple/quad...(s).
// Reference: https://www.sudoku9x9.com/howtosolve/expert/
// https://www.sudokuonline.io/tips/advanced-sudoku-strategies
void solveHiddens(Grid *grid, int **grid_vals) {
  //
}

// Naked single: When a cell only has 1 candidate.
void solveNakedSingles(Grid *grid, int **grid_vals) {
  // Candidate Counts (_cc) and indexes for each house
  int b_cc, b_idx, r_idx, c_idx;
  int *candidates;

  int box_size = (int)sqrt(grid->order);
  int changes_made;

  while (!grid->complete && grid->valid) {
    changes_made = 0;

    for (b_idx = 0; b_idx < grid->order; b_idx++) {
      b_cc = grid->houses[box_t_idx][b_idx][0];

      if (b_cc > 0) {
        int start_row = (b_idx / box_size) * box_size;
        int start_col = (b_idx % box_size) * box_size;

        for (r_idx = start_row; r_idx < start_row + box_size; r_idx++) {
          for (c_idx = start_col; c_idx < start_col + box_size; c_idx++) {
            if (grid_vals[r_idx][c_idx] == 0) {
              candidates = locked_candidates(grid->houses[box_t_idx][b_idx],
                                             grid->houses[row_t_idx][r_idx],
                                             grid->houses[col_t_idx][c_idx]);

              if (candidates == NULL) {
                grid->valid = false;
                break;
              }
              
              if (candidates[0] == 1) {
                setValue(candidates[1], grid, r_idx, c_idx);
                grid_vals[r_idx][c_idx] = candidates[1];
                changes_made++;
              }

              free(candidates);
            }
          }
        }
      }
    }
    checkGridStatus(grid);

    if (!(grid->valid)) {
      printf("solveCandidates ERROR: Grid invalidated during solve.\n");
      break;
    }

    if (changes_made < 1) {
      break; // If no changes made, no progress can be made
      //  @TODO: REMOVE THIS AFTER HIDDEN PAIRS/TRIPLES/QUADS CHECKING.
    }

    // printSudokuPuzzle(grid_vals, grid->order);
  }
}

// expects file name of the puzzle as argument in command line
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./sudoku puzzle.txt\n");
    return EXIT_FAILURE;
  }

  Grid *grid = newGrid();

  int **grid_vals = readSudokuPuzzle(argv[1], grid);
  checkGridStatus(grid);

  if (!grid->valid) {
    printSudokuPuzzle(grid_vals, grid->order);

    deleteSudokuPuzzle(grid, grid_vals);
    return EXIT_FAILURE;

  } else {
    printf("Puzzle valid.\n");
  }

  if (!grid->complete) {
    printf("Puzzle incomplete: ");
    printSudokuPuzzle(grid_vals, grid->order);

    printf("Proceeding to solve... \n");
    solveNakedSingles(grid, grid_vals);
  }

  printf("%s", grid->complete ? "Puzzle completed: " : "Puzzle could not be solved:");

  printSudokuPuzzle(grid_vals, grid->order);
  deleteSudokuPuzzle(grid, grid_vals);
  return EXIT_SUCCESS;
}
