/**
 * Conway's Game of Life
 * 
 * This version runs in serial. Compile with:
 *     gcc -Wall -O3 -march=native game_of_life_serial.c helpers.c -o game_of_life_serial
 * And run with:
 * 	   ./game_of_life_serial [-n num-of-iterations] [-r random-seed] [-s world_size]
 */

#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>
#include <sys/mman.h>

#include "helpers.h"


int main(int argc, char* const argv[]) {
	size_t iterations = 3;
	char * input_file = "examples/input.npy";
	char * output_file = "output/out.npy";

	if (argc > 4) { printf("Wrong number of arguments!\n"); return 1; }
	if (argc == 2) {
		iterations = atoi(argv[1]);
		if (iterations <= 0) { fprintf(stderr, "Must specify a positive number of iterations\n"); return 1; }
	} else if (argc == 3) {
		input_file = argv[1];
		output_file = argv[2];
	} else {
		iterations = atoi(argv[1]);
		input_file = argv[2];
		output_file = argv[3];
	}

	size_t m, n;
	uint8_t* grid = grid_from_npy_path(input_file, &m, &n);
	if (!grid) { perror("grid_from_npy_path(grid)"); return 1; }

	size_t grid_size = m * n;
	uint8_t* grid_copy = (uint8_t*) malloc(grid_size*sizeof(uint8_t));
	memcpy(grid_copy, grid, grid_size);

	uint8_t* grid_next = (uint8_t*) malloc(grid_size*sizeof(uint8_t));
	uint8_t *grid_next_orig = grid_next, *grid_orig = grid_copy;

	uint8_t* grids = (uint8_t*) malloc((iterations+1)*grid_size*sizeof(uint8_t));
	// grids[0] = *grid_copy;

	// Use this for the same simulation as the original
	// for (size_t i = 145; i < 155; i++) grid[i] = ALIVE;
	// grid[282] = 1;
	// grid[283] = 1;
	// grid[284] = 1;
	// grid[285] = 1;
	// grid[301] = 1;
	// grid[305] = 1;
	// grid[325] = 1;
	// grid[341] = 1;
	// grid[344] = 1;

	struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

	for (size_t step = 0; step < iterations; step++) {
		// system("clear");
		// print_world(grid, world_size);
		for (size_t i = 0; i < grid_size; i++) {
			// if (i % world_size == 0) printf("\n\n");
			// if (grid[i]) printf("\t*");
			// else printf("\t");
			size_t num_neighbors = get_num_neighbors(grid_copy, i, n, grid_size);
			if (grid_copy[i]) { grid_next[i] = num_neighbors <= 1 || num_neighbors > 3 ? DEAD : ALIVE; }
			else { grid_next[i] = num_neighbors == 3 ? ALIVE : DEAD; }
		}

		swap(&grid_copy, &grid_next);
		grids[step+1] = *grid_copy;
		// print_world(grid_copy, n);
		
		// print_world(&grids[step+1], n);
		// printf("\n\n");
		// system("sleep 0.5");
  	}

	clock_gettime(CLOCK_MONOTONIC, &end);
    double time = end.tv_sec-start.tv_sec+(end.tv_nsec-start.tv_nsec)/1000000000.0;
    printf("Time: %g secs\n", time);

	// for (size_t i = 0; i < iterations + 1; i++) {
	// 	print_world(grids[i], n);
	// }
	// grid_to_npy_path(output_file, grid, m, n);

	size_t addr = ((size_t)grid_orig) & ~(sysconf(_SC_PAGE_SIZE)-1);
	munmap((void*)addr, grid_size*sizeof(uint8_t));
	free(grid_next_orig);
	free(grid_copy);
  	return 0;
}
