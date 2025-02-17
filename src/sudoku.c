// Sudoku puzzle verifier and solver

#include "grid.h"
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int ORDER;

pthread_mutex_t mutex;

// takes filename and pointer to Grid of Houses
// returns grid values of Sudoku puzzle and fills houses
int **readSudokuPuzzle(char *filename, Grid *grid) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s\n", filename);
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  if (strstr(filename, ".txt") == NULL) {
    fprintf("Invalid file: File %s must be a text file.\n", filename);
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  int psize;
  fscanf(fp, "%d", &psize);

  // Check that number is perfect square
  int root = (int)sqrt(psize);
  if (psize <= 0 || (root * root) != psize) {
    grid->houses[0][0][0] = -1;
    printf("Invalid puzzle size %d: Puzzle must be a perfect square (2x2 "
           "(size-4), 3x3 "
           "(size-9), 4x4 (size 16) etc).",
           psize);
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

  if (!(grid->valid) && !(grid->complete)) {
    free(grid_it);
    return NULL;
  } // Grid alrdy known to be invalid from other threads.

  // Don't need to lock for reading
  for (int hs_idx = 0; hs_idx < grid->order; hs_idx++) {
    if (grid->houses[hs_t_idx][hs_idx][0] == INVALID && grid->valid) {
      // Lock before changing shared invalid bool
      pthread_mutex_lock(&mutex);
      grid->valid = false;
      pthread_mutex_unlock(&mutex);

      printf("Invalid puzzle: A house should not have duplicate values.\n");
      // printf("Found duplicate: hs_t_idx=%d, hs_idx=%d.\n", hs_t_idx, hs_idx);
      //  printf("House counts: ");
      //  printCounts(grid->houses[hs_t_idx][hs_idx]);
      //  printf("\n");

    } else if (grid->houses[hs_t_idx][hs_idx][0] != FULL && grid->complete) {
      // Locking before changing shared complete bool
      pthread_mutex_lock(&mutex);
      grid->complete = false;
      pthread_mutex_unlock(&mutex);
    }
  }

  free(grid_it);
  return NULL;
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

  if (pthread_mutex_init(&mutex, NULL) != 0) {
    printf("checkGridStatus Error: Mutex initialization failed.\n");
    exit(EXIT_FAILURE);
  }

  // Assume innocent until proven guilty.
  grid->complete = true;
  grid->valid = true;

  pthread_t threads[3];
  for (int i = 0; i < NUM_HOUSE_TYPES; i++) {
    GridIterator *grid_it = newGridTypeIt(grid, i);

    if (pthread_create(&threads[i], NULL, checkHousesStatus, grid_it) != 0) {
      printf("checkHousesStatus Error: pthread_create failed.\n");
      free(grid_it);
      exit(EXIT_FAILURE);
    }
  }

  // Wait for all threads to complete
  for (int i = 0; i < NUM_HOUSE_TYPES; i++) {
    pthread_join(threads[i], NULL);
  }
}

// Hidden single: A house has multiple candidates, but
//                only 1 cell can house a certain candidate.
// Any house type can have hidden single/pair/triple/quad...(s).
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

  while (!(grid->complete)) {
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

              if (candidates[0] == 1) {
                setValue(candidates[1], *grid, r_idx, c_idx);
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
    pthread_mutex_destroy(&mutex);
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

  printf("%s",
         grid->complete ? "Puzzle completed: " : "Puzzle could not be solved:");

  printSudokuPuzzle(grid_vals, grid->order);
  deleteSudokuPuzzle(grid, grid_vals);
  pthread_mutex_destroy(&mutex);
  return EXIT_SUCCESS;
}
