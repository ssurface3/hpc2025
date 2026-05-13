#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
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

    int TOTAL_CELLS = 120;
    
    int rule_number = 30; 
    
    int generations = 60;
    int is_periodic = 1;

    int ndim = 1;
    int dims[1] = {world_size};
    int periods[1] = {is_periodic};
    int reorder = 1;

    MPI_Comm cart_comm;
    MPI_Cart_create(MPI_COMM_WORLD, ndim, dims, periods, reorder, &cart_comm);

    int n_r[2]; 
    MPI_Cart_shift(cart_comm, 0, 1, &n_r[0], &n_r[1]);

    int local_n = TOTAL_CELLS / world_size;
    std::vector<int> current_grid(local_n + 2, 0);
    std::vector<int> next_grid(local_n + 2, 0);

    if (world_rank == world_size / 2) {
        current_grid[local_n / 2 + 1] = 1; 
    }

    std::vector<int> global_grid;
    if (world_rank == 0) {
        global_grid.resize(TOTAL_CELLS);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    for (int g = 0; g < generations; g++) {
        MPI_Gather(&current_grid[1], local_n, MPI_INT,
                   global_grid.data(), local_n, MPI_INT,
                   0, MPI_COMM_WORLD);
        if (world_rank == 0) {
            for (int i = 0; i < TOTAL_CELLS; i++) {
                if (global_grid[i] == 1) {
                    printf("#"); 
                } else {
                    printf(" "); 
                }
            }
            printf("\n");
        }

        MPI_Sendrecv(&current_grid[local_n],     1, MPI_INT, n_r[1], 0,
                     &current_grid[0],           1, MPI_INT, n_r[0], 0,
                     cart_comm, MPI_STATUS_IGNORE);

        MPI_Sendrecv(&current_grid[1],           1, MPI_INT, n_r[0], 1,
                     &current_grid[local_n + 1], 1, MPI_INT, n_r[1], 1,
                     cart_comm, MPI_STATUS_IGNORE);

        if (is_periodic == 0) {
            if (n_r[0] == MPI_PROC_NULL) current_grid[0] = 0;
            if (n_r[1] == MPI_PROC_NULL) current_grid[local_n + 1] = 0;
        }

        for (int i = 1; i <= local_n; i++) {
            next_grid[i] = apply_rule(current_grid[i-1], current_grid[i], current_grid[i+1], rule_number);
        }

        for (int i = 1; i <= local_n; i++) {
            current_grid[i] = next_grid[i];
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    if (world_rank == 0) {
        printf("\nTime taken: %f seconds\n", end_time - start_time);
    }

    MPI_Comm_free(&cart_comm);
    MPI_Finalize();
    return 0;
}