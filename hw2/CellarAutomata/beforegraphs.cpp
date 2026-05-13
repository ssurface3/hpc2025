#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

int apply_rule(int left, int center, int right, int rule) {
    int state = (left << 2) | (center << 1) | right;
    return (rule >> state) & 1;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int TOTAL_CELLS = 64;       
    int rule_number = 30;       
    int generations = 10;       
    int is_periodic = 1;        

    int ndim = 1;
    int dims[1] = {world_size}; 
    int periods[1] = {is_periodic}; 
    int reorder = 1;

    MPI_Comm cart_comm;
    MPI_Cart_create(MPI_COMM_WORLD, ndim, dims, periods, reorder, &cart_comm);

    enum DIRECTIONS {LEFT, RIGHT};
    int n_r[2];
    MPI_Cart_shift(cart_comm, 0, 1, &n_r[LEFT], &n_r[RIGHT]);

    int local_n = TOTAL_CELLS / world_size; 
    
    std::vector<int> current_grid(local_n + 2, 0);
    std::vector<int> next_grid(local_n + 2, 0);

    srand(time(NULL) + world_rank);
    for (int i = 1; i <= local_n; i++) {
        current_grid[i] = rand() % 2;
    }

    for (int g = 0; g < generations; g++) {
        
        MPI_Sendrecv(&current_grid[local_n],     1, MPI_INT, n_r[RIGHT], 0,
                     &current_grid[local_n + 1], 1, MPI_INT, n_r[RIGHT], 0,
                     cart_comm, MPI_STATUS_IGNORE);

        MPI_Sendrecv(&current_grid[1], 1, MPI_INT, n_r[LEFT], 1,
                     &current_grid[0], 1, MPI_INT, n_r[LEFT], 1,
                     cart_comm, MPI_STATUS_IGNORE);

        if (is_periodic == 0) {
            if (n_r[LEFT] == MPI_PROC_NULL)  current_grid[0] = 0;
            if (n_r[RIGHT] == MPI_PROC_NULL) current_grid[local_n + 1] = 0;
        }

        for (int i = 1; i <= local_n; i++) {
            next_grid[i] = apply_rule(current_grid[i-1], current_grid[i], current_grid[i+1], rule_number);
        }

        for (int i = 1; i <= local_n; i++) {
            current_grid[i] = next_grid[i];
        }
    }

    MPI_Comm_free(&cart_comm);
    MPI_Finalize();
    return 0;
}