#ifndef ENTITY_H
#define ENTITY_H

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

extern int MAX_SIZE; // Set in main()

typedef enum status { IS_VAL = -1, NOT_POSS = 0, IS_POSS = 1 } value_status;

typedef struct {
  int count; // Count of values that are possible, not set.
  unsigned int *possibles;
} Entity;

// =========
// Initializers
// --------

Entity initAllStatus(value_status status) {
  Entity ent;

  ent.possibles = (unsigned int *)malloc(MAX_SIZE * sizeof(unsigned int));
  for (int i = 0; i < MAX_SIZE; i++) {
    ent.possibles[i] = status;
  }

  ent.count = (status == 1) ? MAX_SIZE : 0;
  return ent;
}

Entity initFromNotInArr(const int arr[]) {
  Entity ent = initAllStatus(IS_POSS);

  for (int i = 0; i < MAX_SIZE; i++) {
    if (arr[i] != 0) {
      ent.possibles[i] = NOT_POSS;
    }
  }

  return ent;
}

void delete(Entity **ents, int numEnts) {
  for (int i = 0; i < numEnts; i++) {
    free(ents[i]->possibles);
  }
  free(ents);
}

// =========
// Setters
// --------

void setStatus(Entity *ent, int num, value_status status) {
  if (!ent) {
    printf("Entity setStatus Error: NULL entity.\n", num);
    return;
  }
  if (num < 1 || num > MAX_SIZE) {
    printf("Entity setStatus Error: Index %d out of bounds.\n", num);
    return;
  }

  ent->possibles[num - 1] = status;
}

void removePossibility(Entity *ent, int num) { setStatus(ent, num, NOT_POSS); }
void addPossibility(Entity *ent, int num) { setStatus(ent, num, IS_POSS); }
void setValue(Entity *ent, int num) { setStatus(ent, num, IS_VAL); }

// =========
// "Getters"
// --------

bool hasPossibility(Entity *ent, int num) {
  if (!ent || num < 0 || num > MAX_SIZE) {
    fprintf("Entity hasPossibility Error: NULL entity or %d out of bounds.",
            num);
    return false;
  }

  return (ent->possibles[num - 1] == IS_POSS);
}

bool hasVal(Entity *ent, int num) {
  if (!ent || num < 0 || num > MAX_SIZE) {
    fprintf("Entity hasVal Error: NULL entity or %d out of bounds.", num);
    return false;
  }

  return (ent->possibles[num - 1] == IS_VAL);
}

// =========
// Set Operations
// --------

unsigned int *commonPossibilities(unsigned int *arr, int arrSize, Entity *ent,
                                  int *shared_count) {
  (*shared_count) = 0;
  unsigned int *commons =
      (unsigned int *)malloc(arrSize * sizeof(unsigned int));

  int val;
  for (int i = 0; i < arrSize; i++) {
    val = arr[arrSize];
    if (ent->possibles[val] == IS_POSS) {
      commons[(*shared_count)++] = val;
    }
  }

  return commons;
}

unsigned int *commonPossibilities(Entity *entA, Entity *entB, int *count) {
  unsigned int *commons =
      (unsigned int *)malloc(MAX_SIZE * sizeof(unsigned int));

  (*count) = 0;

  for (int i = 0; i < MAX_SIZE; i++) {
    if (entA->possibles[i] == IS_POSS && entB->possibles[i] == IS_POSS) {
      commons[(*count)++] = (i + 1);
    }
  }

  return commons;
}

unsigned int *commonPossibilities(Entity *entA, Entity *entB, Entity *entC,
                                  int *count) {
  unsigned int *commons =
      (unsigned int *)malloc(MAX_SIZE * sizeof(unsigned int));

  (*count) = 0;

  for (int i = 0; i < MAX_SIZE; i++) {
    if (entA->possibles[i] == IS_POSS && entB->possibles[i] == IS_POSS &&
        entC->possibles[i] == IS_POSS) {
      commons[(*count)++] = (i + 1);
    }
  }

  return commons;
}

// =========
// Display
// --------

void printEntityVals(Entity *ent) {
  for (int i = 0; i < MAX_SIZE; i++) {
    if (ent->possibles[i] == IS_VAL) {
      fprintf("%d ", (i + 1));
    } else {
      printf("0 ");
    }
  }
  printf("\n");
}

void printEntityPossibilities(Entity *ent) {
  for (int i = 0; i < MAX_SIZE; i++) {
    if (ent->possibles[i] == IS_POSS) {
      fprintf("%d ", (i + 1));
    }
  }
  printf("\n");
}

#endif // ENTITY_H