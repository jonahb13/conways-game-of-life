#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <ctype.h>

#include "helpers.h"
#include "matrix_io_helpers.h"

/**
 * Make the current grid the new grid
 */
void swap(uint8_t** grid, uint8_t** grid_next) {
    uint8_t* temp = *grid;
    *grid = *grid_next;
    *grid_next = temp;
}


/**
 * Gets the number of live organisms around a given position 
 */
int get_num_neighbors(uint8_t* grid, const size_t i, const size_t world_size, const size_t grid_size) {
	int neighbor_count = 0;
	int before_positions[4] = {i - world_size - 1, i - world_size, i - world_size + 1, i - 1};
	int after_positions[4] = {i + 1, i + world_size - 1, i + world_size, i + world_size + 1};
	
	for (size_t j = 0; j < 4; j++) {
		if (before_positions[j] >= 0) {
			if (grid[before_positions[j]]) neighbor_count++;
		}
		if (after_positions[j] < grid_size) {
			if (grid[after_positions[j]]) neighbor_count++;
		}
	}

  return neighbor_count;
}


void print_world(uint8_t* grid, size_t world_size) {
	for (size_t i = 0; i < world_size; i++) {
		for (size_t j = 0; j < world_size; j++) {
			printf("%hhu, ", grid[i*world_size+j]);
		}
		printf("\n");
	}
    printf("\n");
}

/**
 * Creates a new matrix by loading the data from the given NPY file. This is
 * a file format used by the numpy library. This function only supports arrays
 * that are little-endian doubles, c-contiguous, and 1 or 2 dimensional. The
 * file is loaded as memory-mapped so it is backed by the file and loaded
 * on-demand. The file should be opened for reading or reading and writing.
 * 
 * This will return NULL if the data cannot be read, the file format is not
 * recognized, there are memory allocation issues, or the array is not a
 * supported shape or data type.
 */
uint8_t* grid_from_npy(FILE* file, size_t *m, size_t *n) {
    // Read the header, check it, and get the shape of the matrix
    size_t sh[2], offset;
    if (!__npy_read_header(file, sh, &offset)) { return NULL; }
    
    // Get the memory mapped data
    void* x = (void*)mmap(NULL, sh[0]*sh[1] + offset,
                          PROT_READ|PROT_WRITE, MAP_SHARED, fileno(file), 0);
    if (x == MAP_FAILED) { return NULL; }

    // Make the matrix itself
    uint8_t* data = (uint8_t*)(((char*)x) + offset);
    *m = sh[0];
    *n = sh[1];
    return data;
}

/**
 * Same as matrix_from_npy() but takes a file path instead.
 */
uint8_t* grid_from_npy_path(const char* path, size_t *m, size_t *n) {
    FILE* f = fopen(path, "r+b");
    if (!f) { return NULL; }
    uint8_t* grid = grid_from_npy(f, m, n);
    fclose(f);
    return grid;
}

// /**
//  * Saves a matrix to a CSV file.
//  * 
//  * If the file argument is given as stdout, this will print it to the terminal.
//  */
// void matrix_to_csv(FILE* file, const uint8_t* grid, size_t m, size_t n) {
//     printf("rows %lu, cols %lu", m, n);
//     if (m < 1 || n < 1) { return; }
//     for (size_t i = 0; i < m; i++) {
//         for (size_t j = 0; j < n; j++) {
//             fprintf(file, "%hhu,", grid[i*n+j]);
//         }
//         fprintf(file, "\n");
//     }
// }

// /**
//  * Same as matrix_to_csv() but takes a file path instead.
//  */
// bool matrix_to_csv_path(const char* path, const uint8_t* grid, size_t m, size_t n) {
//     FILE* f = fopen(path, "w");
//     if (!f) { return false; }
//     matrix_to_csv(f, grid, m, n);
//     fclose(f);
//     return true;
// }

/**
 * Saves a matrix to a NPY file. This is a file format used by the numpy
 * library. This will return false if the data cannot be written.
 */
bool grid_to_npy(FILE* file, const uint8_t* grid, size_t m, size_t n) {
    // create the header
    char header[128];
    size_t len = snprintf(header, sizeof(header), "\x93NUMPY\x01   "
        "{'descr': '<u1', 'fortran_order': False, 'shape': (%zu, %zu), }",
        m, n);
    if (len < 0) { return false; }
    header[7] = 0; // have to after the string is written
    *(unsigned short*)&header[8] = sizeof(header) - 10;
    memset(header + len, ' ', sizeof(header)-len-1);
    header[sizeof(header)-1] = '\n';

    // write the header and the data
    bool head = fwrite(header, 1, sizeof(header), file) == sizeof(header);
    if (!head) return false;
    
    return fwrite(grid, sizeof(uint8_t), n*m, file) == n*m;
}

/**
 * Same as matrix_to_npy() but takes a file path instead.
 */
bool grid_to_npy_path(const char* path, const uint8_t* grid, size_t m, size_t n) {
    FILE* f = fopen(path, "wb");
    if (!f) { return false; }
    grid_to_npy(f, grid, m, n);
    fclose(f);
    return true;
}

