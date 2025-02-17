#ifndef ENTITYMAP_H
#define ENTITYMAP_H

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

extern int MAX_POSSIBILITIES; // Set in main()

#if MAX_POSSIBILITIES > 32
#error "MAX_POSSIBILITIES cannot be larger than 32."
#endif

typedef struct {
  unsigned int possibles; // bit (n-1) represents the possibility of number n.
  int count;              // Count of elements in the set.
} Entity;

void removePossibility(Entity *ent, int num) {
  if (num < 1 || num > MAX_POSSIBILITIES) {
    printf("removePossibility Error: %d out of bounds.\n", num);
    return;
  }
  int bitPos = num - 1;
  if (ent->possibles & (1U << bitPos)) {
    ent->possibles &= ~(1U << bitPos);
    ent->count--;
  }
}

void addPossibility(Entity *ent, int num) {
  if (num < 1 || num > MAX_POSSIBILITIES) {
    printf("removePossibility Error: %d out of bounds.\n", num);
    return;
  }
  int bitPos = num - 1;
  if (!(ent->possibles & (1U << bitPos))) {
    ent->possibles |= (1U << bitPos);
    ent->count++;
  }
}

Entity initAllPossible() {
  Entity ent = {(1 << MAX_POSSIBILITIES) - 1, MAX_POSSIBILITIES};
  return ent;
}

Entity initAllPossible(int size) {
  Entity ent = {(1 << size) - 1, size};
  return ent;
}

Entity initFromNotInArr(const int arr[], int size) {
  Entity ent = initAllPossible();

  for (int i = 0; i < size; i++) {
    if (arr[i] != 0) {
      removePossibility(&ent, i);
    }
  }

  return ent;
}

Entity initFromInArr(const int arr[], int size) {
  Entity ent = {0, 0};

  for (int i = 0; i < size; i++) {
    if (arr[i] != 0) {
      addPossibility(&ent, i);
    }
  }
  return ent;
}

bool samePossibilities(Entity *entA, Entity *entB) {
  if (!entA || !entB) {
    return false;
  }
  return (entA->count == entB->count) && (entA->possibles == entB->possibles);
}

bool samePossibilities(Entity *entA, Entity *entB, Entity *entC) {
  if (!entA || !entB || !entC) {
    return false;
  }
  return (entA->count == entB->count == entC->count) &&
         (entA->possibles == entB->possibles == entC->possibles);
}

int *commonPossibilities(Entity *entA, Entity *entB) {
  if (!entA || !entB) {
    printf("commonPoss Error: NULL Entity pointer(s).\n");
    return;
  }

  unsigned int commonPossibilities = entA->possibles & entB->possibles;
  return bitmaskToInts(commonPossibilities);
}

int *commonPossibilities(Entity *entA, Entity *entB, Entity *entC) {
  if (!entA || !entB || !entC) {
    printf("commonPoss Error: NULL Entity pointer(s).\n");
    return;
  }

  unsigned int commonPossibilities =
      entA->possibles & entB->possibles & entC->possibles;
  return bitmaskToInts(commonPossibilities);
}

int *bitmaskToInts(unsigned int bitmask, int *size) {
  int *values = (int *)malloc((*size) * sizeof(int));
  if (!values) {
    printf("bitmaskToInts Error: Memory allocation failed.\n");
    return NULL;
  }
  int count = 0;
  for (int i = 0; i < size; i++) {
    if (bitmask & (1u << i)) {
      values[count++] = i + 1;
    } else {
      (*size)--;
    }
  }
  return values;
}

int *bitmaskToInts(unsigned int bitmask) {
  int *values = (int *)malloc(MAX_POSSIBILITIES * sizeof(int));
  if (!values) {
    printf("bitmaskToInts Error: Memory allocation failed.\n");
    return NULL;
  }
  int count = 0;
  for (int i = 0; i < MAX_POSSIBILITIES; i++) {
    if (bitmask & (1u << i)) {
      values[count++] = i + 1;
    }
  }
  return values;
}

int *getPossibleValues(Entity *ent) { return bitmaskToInts(ent->possibles); }

#endif // ENTITYMAP_H