/**
 * Conway's Game of Life using Cuda
 * 
 * This version runs in parallel on a GPU using Cuda. Compile with:
 * 	   gcc -Wall -O3 -march=native -c helpers.c
 *     nvcc -arch=sm_20 -O3 game_of_life_cuda.cu helpers.o -o game_of_life_cuda -lm
 * And run with:
 * 	   ./game_of_life_cuda [-n num-of-iterations] [-r random-seed] [-s world_size]
 */

 #include<stdio.h>
 #include<stdlib.h>
 #include <unistd.h>
 #include <inttypes.h>
 #include <time.h>
 #include <string.h>
 #include <cuda_runtime.h>


 #define CHECK(call)                                                       \
{                                                                         \
   const cudaError_t error = call;                                        \
   if (error != cudaSuccess)                                              \
   {                                                                      \
      printf("Error: %s:%d, ", __FILE__, __LINE__);                       \
      printf("code:%d, reason: %s\n", error, cudaGetErrorString(error));  \
      exit(1);                                                            \
   }                                                                      \
}

void print_world(size_t* grid, size_t world_size) {
	for (size_t i = 0; i < world_size; i++) {
		for (size_t j = 0; j < world_size; j++) {
			printf("%lu, ", grid[i*world_size+j]);
		}
		printf("\n");
	}
    printf("\n");
}

__device__
int get_num_neighbors(size_t* grid, const size_t i, const size_t world_size, const size_t grid_size) {
	const int half_neighbors = 4;
    size_t neighbor_count = 0;
    int before_positions[half_neighbors] = {i - world_size - 1, i - world_size, i - world_size + 1, i - 1};
    int after_positions[half_neighbors] = {i + 1, i + world_size - 1, i + world_size, i + world_size + 1};
    
    for (size_t j = 0; j < half_neighbors; j++) {
        if (before_positions[j] >= 0) {
            if (grid[before_positions[j]]) neighbor_count++;
        }
        if (after_positions[j] < grid_size) {
            if (grid[after_positions[j]]) neighbor_count++;
        }
    }
 
   return neighbor_count;
 }

 /**
 * Make the current position array the next position array
 */
 __device__
void swap(size_t** grid, size_t** grid_next) {
    size_t* temp = *grid;
    *grid = *grid_next;
    *grid_next = temp;
}

__global__
void simulate(uint8_t* grid, size_t* grid_next, const size_t world_size, const size_t iterations) {
	const int i = threadIdx.x, grid_size = blockDim.x;
    for (size_t step = 0; step < iterations; step++) {
		for (j += blockDim.x) {
			// i+j*blockDim.x;
			size_t num_neighbors = get_num_neighbors(grid, i, world_size, grid_size);
			if (grid[i]) { grid_next[i] = num_neighbors <= 1 || num_neighbors > 3 ? 0 : 1; }
			else { grid_next[i] = num_neighbors == 3 ? 1 : 0; }
		}

		swap(&grid, &grid_next);
		__syncthreads();
  	}
}


int main(int argc, char* const argv[]) {
	size_t iterations = 1;
	size_t random_seed = time(NULL);
    size_t world_size = 20;
 
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
	const size_t grid_bytes = grid_size*sizeof(size_t);
	size_t* h_grid = (size_t*) malloc(grid_bytes);
	size_t* h_grid_next = (size_t*) malloc(grid_bytes);
	memset(h_grid, 0, grid_bytes);

	size_t *d_grid, *d_grid_next;
 
     // Use this for random data
    //  srand(random_seed);
    //  for (size_t i = 0; i < grid_size; i++) h_grid[i] = rand() & 1;
 
     // Use this for the same simulation as the original
    for (size_t i = 145; i < 155; i++) h_grid[i] = 1;
    h_grid[282] = 1;
    h_grid[283] = 1;
    h_grid[284] = 1;
    h_grid[285] = 1;
    h_grid[301] = 1;
    h_grid[305] = 1;
    h_grid[325] = 1;
    h_grid[341] = 1;
    h_grid[344] = 1;

    CHECK(cudaMalloc(&d_grid, grid_bytes));
    CHECK(cudaMalloc(&d_grid_next, grid_bytes));

    CHECK(cudaMemcpy(d_grid, h_grid, grid_bytes, cudaMemcpyHostToDevice));
 
    simulate<<<1, grid_size>>>(d_grid, d_grid_next, world_size, iterations);
    CHECK(cudaDeviceSynchronize());

    CHECK(cudaMemcpy(h_grid_next, d_grid_next, grid_bytes, cudaMemcpyDeviceToHost));

    print_world(h_grid_next, world_size);
    free(h_grid); free(h_grid_next);
    CHECK(cudaFree(d_grid)); CHECK(cudaFree(d_grid_next));
    return 0;
}
 