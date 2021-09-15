#include <stdlib.h>
#include <stdbool.h>

#define DEAD  0
#define ALIVE 1

/**
 * Make the current grid the new grid
 */
void swap(uint8_t** grid, uint8_t** grid_next);

/**
 * Gets the number of live organisms around a given position 
 */
int get_num_neighbors(uint8_t* grid, size_t i, size_t world_size, size_t grid_size);

void print_world(uint8_t* grid, size_t world_size);

uint8_t* grid_from_npy(FILE* file, size_t* m, size_t* n);

uint8_t* grid_from_npy_path(const char* path, size_t* m, size_t* n);

bool grid_to_npy(FILE* file, const uint8_t* grid, size_t m, size_t n);

bool grid_to_npy_path(const char* path, const uint8_t* grid, size_t m, size_t n);

