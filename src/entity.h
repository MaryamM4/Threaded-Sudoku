
#ifndef ENTITY_H
#define ENTITY_H

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// MIN credit: https://stackoverflow.com/a/3437433
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MIN3(x, y, z) (MIN(MIN((x), (y)), (z))) // MIN3: ChatGPT

#define NUM_HOUSE_TYPES 3
typedef enum { box_t_idx = 0, row_t_idx, col_t_idx } HOUSE_T_IDX;

extern int ORDER; // Number of cols/rows in the grid

typedef enum status { INVALID = -1, FULL = 0 } status;

// Idx 0 of a House is reserved for a candidate count
// Every other idx represents a value, and stores a count
// of the number of times it appear in the House.
// E.g., if House[3] = 1, there is one '3' in the House.

// Idx 0: Count of total possible. Idx 1-ORDER, count of value within house.
typedef int *House;    // Box/col/row. 1-idxed (0 reserved for candidate count)
typedef House *Houses; // 0-indexed

typedef struct {
  int order;                      // Number of cols/rows/boxes in grid.
  Houses houses[NUM_HOUSE_TYPES]; // 0: boxes, 1: rows, 2: col houses.
  bool valid, complete;
} Grid;

// Needed for thread arguments.
typedef struct {
  Grid *grid;
  int house_type_idx, house_idx;
} GridIterator;

// =========
// "De/Constructors"
// --------

House initFullHouse() {
  House hs = (House)malloc((ORDER + 1) * sizeof(int));
  if (hs == NULL) {
    perror("initEmptyHouse Error: Failed to allocate memory.");
    exit(EXIT_FAILURE);
  }

  hs[0] = 0; // No candidates possible.
  for (int val = 1; val <= ORDER; val++) {
    hs[val] = 1;
  }

  return hs;
}

House initEmptyHouse() {
  House hs = (House)malloc((ORDER + 1) * sizeof(int));
  if (hs == NULL) {
    perror("initEmptyHouse Error: Failed to allocate memory.");
    exit(EXIT_FAILURE);
  }

  hs[0] = ORDER;
  for (int val = 1; val <= ORDER; val++) {
    hs[val] = 0;
  }

  return hs;
}

Houses initEmptyHouses() {
  Houses hss = (Houses)malloc(ORDER * sizeof(House));
  if (hss == NULL) {
    perror("initEmptyHouses Error: Failed to allocate memory.");
    exit(EXIT_FAILURE);
  }

  for (int hs_idx = 0; hs_idx < ORDER; hs_idx++) {
    hss[hs_idx] = initEmptyHouse();
  }

  return hss;
}

Grid *newGrid() {
  Grid *grid = (Grid *)malloc(sizeof(Grid));
  if (!grid) {
    printf("newGrid Error: Failed to allocate grid.");
    exit(EXIT_FAILURE);
  }

  grid->order = 0;
  grid->valid = false;
  grid->complete = false;

  for (int i = 0; i < NUM_HOUSE_TYPES; i++) {
    grid->houses[i] = NULL;
  }
  return grid;
}

GridIterator *newGridIt(Grid *grid, int hs_t_idx, int hs_idx) {
  if (hs_t_idx < -1 || hs_t_idx >= NUM_HOUSE_TYPES) {
    printf("newGridIt Error: house_type_index %d out of bounds. Must be -1, or "
           "0-%d.\n",
           hs_t_idx, NUM_HOUSE_TYPES - 1);
    exit(EXIT_FAILURE);
  }

  if (hs_idx < -1 || hs_idx >= ORDER) {
    printf(
        "newGridIt Error: house_index %d out of bounds. Must be -1, or 0-%d.\n",
        hs_idx, ORDER - 1);
    exit(EXIT_FAILURE);
  }

  GridIterator *grid_it = (GridIterator *)malloc(sizeof(GridIterator));
  if (!grid_it) {
    printf("newGridIt Error: Failed to allocate memory for GridIterator.\n");
    exit(EXIT_FAILURE);
  }

  grid_it->grid = grid;
  grid_it->house_type_idx = hs_t_idx;
  grid_it->house_idx = hs_idx;

  return grid_it;
}

// Through iterating through house types
GridIterator *newGridTypeIt(Grid *grid, int hs_t_idx) {
  return newGridIt(grid, hs_t_idx, -1);
}

// For iterating through indexes in houses
GridIterator *newGridHouseIt(Grid *grid, int hs_idx) {
  return newGridIt(grid, -1, hs_idx);
}

void del(Houses *hss) {
  for (int i = 0; i < ORDER; i++) {
    free((*hss)[i]);
    (*hss)[i] = NULL;
  }
  free(*hss);
  *hss = NULL;
}

void delGrid(Grid *grid) {
  for (int i = 0; i < NUM_HOUSE_TYPES; i++) {
    del(&(grid->houses[i]));
  }
  free(grid);
}

// =========
// Helper methods
// --------

// Helper method for getting idx of square Entity given corresponding row/col
int getBoxIdx(int row, int col, int size) {
  int box_size = (int)sqrt((double)size);
  return (row / box_size) * box_size + (col / box_size);
}

// =========
// Manipulators
// --------

void addValue(House hs, int val) {
  if (!hs) {
    printf("addValue error: NULL House.\n");
    return;
  }

  if (val < 0 || val > ORDER) {
    printf("addValue warning: %d out of bounds.\n", val);
    hs[0] = INVALID;

  } else if (val > 0) {
    hs[val] += 1; // If not duplicate, House has one less candidate.
    hs[0] = (hs[0] == INVALID || hs[val] > 1) ? INVALID : hs[0] - 1;
  }
}

void removeValue(House hs, int val) {
  if (!(hs) || val > ORDER) {
    printf("addValue Error: NULL House.");
    return;
  }

  if (val < 0 || val > ORDER) {
    printf("removeValue warning:  %d out of bounds.\n", val);
    hs[0] = INVALID;

  } else if (val > 0) {
    hs[val] -= 1; // If valid, House has one more candidate.
    hs[0] = (hs[0] == INVALID || hs[val] > 1) ? INVALID : hs[0] + 1;
  }
}

// When a candidate is determined as an answer
// Add value to corresponding houses.
void setValue(int val, Grid grid, int row, int col) {
  int box = getBoxIdx(row, col, grid.order);

  addValue(grid.houses[box_t_idx][box], val);
  addValue(grid.houses[row_t_idx][row], val);
  addValue(grid.houses[col_t_idx][col], val);
}

// =========
// Set Operations
// --------

// In sudoku, a locked candidate is when a canditade is restricted to a specific
// row/col in a box, and it can be eliminated from other rows/cols out that box.
int *locked_candidate_pairs(House hsA, House hsB) {
  int max_locked = MIN(hsA[0], hsB[0]);

  if (max_locked < 0) {
    perror("locked_candidates Error: Invalid House(s).");
    return NULL;
  }

  int *locked = (int *)malloc((max_locked + 1) * sizeof(int));
  if (!locked) {
    printf("locked_candidate_pairs Error: Failed to allocate memory.");
    exit(EXIT_FAILURE);
  }

  locked[0] = 0; // First index reserved for candidate count.

  for (int val = 1; val <= ORDER; val++) {
    if (hsA[val] == 0 && hsB[val] == 0) {
      locked[locked[0] + 1] = val;
      locked[0]++;
    }
  }
  return locked;
}

void locked_arr_house_cand(int *arr, House hs) {
  int max_locked = MIN(arr[0], hs[0]);

  if (max_locked < 0) {
    printf("locked_arr_house_cand Error: Invalid House(s).");
    arr[0] = -1;
  }

  arr[0] = 0; // First index reserved for candidate count.
  int arr_idx = 1;
  for (int val = 1; val <= ORDER; val++) {
    if (hs[val] == 0) {
      for (int ai = arr_idx; ai < arr[0]; ai++) {
        if (arr[arr_idx] == val) {
          arr[0] += 1;
          arr[arr[0]] = val;
          arr_idx++;
        }
      }
    }
  }
}

int *locked_candidates(House hsA, House hsB, House hsC) {
  int max_locked = MIN3(hsA[0], hsB[0], hsC[0]);

  if (max_locked < 0) {
    perror("locked_candidates Error: Invalid House(s).");
    return NULL;
  }

  int *locked = (int *)malloc((max_locked + 1) * sizeof(int));
  if (!locked) {
    printf("locked_candidate Error: Failed to allocate memory.");
    exit(EXIT_FAILURE);
  }

  locked[0] = 0; // First index reserved for candidate count.

  for (int val = 1; val <= ORDER; val++) {
    if (hsA[val] == 0 && hsB[val] == 0 && hsC[val] == 0) {
      locked[locked[0] + 1] = val;
      locked[0] += 1;
    }
  }

  return locked;
}

// =========
// Display
// --------

void printValues(House hs) {
  int val;
  for (int i = 1; i <= ORDER; i++) {
    val = (hs[i] > 0) ? i : 0;
    printf("%d ", val);
  }

  if (hs[0] == INVALID) {
    printf(" (House INVALID)");
  }

  printf("\n");
}

void printCounts(House hs) {
  for (int i = 1; i <= ORDER; i++) {
    printf("%d ", hs[i]);
  }
  printf("  Candidate count: %d\n", hs[0]);
}

#endif // ENTITY_H