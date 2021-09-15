/**
 * Conway's Game of Life using omp
 * 
 * This version runs in serial. Compile with:
 *     gcc -Wall -O3 -fopenmp -march=native game_of_life_shared.c util.c helpers.c -o game_of_life_shared
 * And run with:
 * 	   ./game_of_life_shared [-n num-of-iterations] [-r random-seed] [-s world_size]
 */

#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#include "util.h"
#include "helpers.h"

int main(int argc, char* const argv[]) {
	size_t iterations = 3;
	size_t random_seed = time(NULL);
	size_t world_size = 20;
    const size_t num_threads = get_num_cores_affinity();

	// parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "n:r:s:")) != -1) {
        char* end;
        switch (opt) {
        case 'n': iterations = strtoumax(optarg, &end, 10); break;
        case 'r': random_seed = strtoul(optarg, &end, 10); break;
        case 's': world_size = strtoumax(optarg, &end, 10); break;
        default:
            fprintf(stderr, "usage: %s [-n num-iterations] [-r random-seed] [-s world-size] input output\n", argv[0]);
            return 1;
        }
    }
    if (optind + 2 < argc || iterations == 0) {
        fprintf(stderr, "usage: %s [-n num-iterations] [-r random-seed] [-s world-size] input output\n", argv[0]);
        return 1;
    }

	const size_t grid_size = world_size * world_size;
    const size_t grid_bytes = grid_size * sizeof(size_t);
	size_t* grid = (size_t*) malloc(grid_bytes);
	size_t* grid_next = (size_t*) malloc(grid_bytes);
	memset(grid, 0, grid_bytes);

	// Use this for random data
	// srand(random_seed);
	// for (size_t i = 0; i < grid_size; i++) grid[i] = rand() & 1;

	// Use this for the same simulation as the original
	for (size_t i = 145; i < 155; i++) grid[i] = 1;
	grid[282] = 1;
	grid[283] = 1;
	grid[284] = 1;
	grid[285] = 1;
	grid[301] = 1;
	grid[305] = 1;
	grid[325] = 1;
	grid[341] = 1;
	grid[344] = 1;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // seed random number generator so all processes generate the same sequence of numbers 
    srand(random_seed);

	for (size_t step = 0; step < iterations; step++) {		
        #pragma omp parallel for num_threads(num_threads)
		for (size_t i = 0; i < grid_size; i++) {
			size_t num_neighbors = get_num_neighbors(grid, i, world_size, grid_size);
			if (grid[i]) { grid_next[i] = num_neighbors <= 1 || num_neighbors > 3 ? DEAD : ALIVE; }
			else { grid_next[i] = num_neighbors == 3 ? ALIVE : DEAD; }
		}
		swap(&grid, &grid_next);
  	}

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time = end.tv_sec-start.tv_sec+(end.tv_nsec-start.tv_nsec)/1000000000.0;
    printf("Time: %g secs\n", time);

    print_world(grid, world_size);

	free(grid); free(grid_next);
  	return 0;
}
