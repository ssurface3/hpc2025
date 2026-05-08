#include <mpi.h>
#include <stdio.h>
#include <vector>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size < 2) {
        if (rank == 0) printf("Error: Run with -n 2\n");
        MPI_Finalize();
        return 1;
    }

    int n_iter = 10000;
    
    if (rank == 0) {
        printf("%-10s | %-7s | %-10s | %-12s | %-10s\n", 
               "Size(B)", "Iters", "Total(s)", "Latency(s)", "MB/s");
        printf("------------------------------------------------------------------\n");
    }

    for (int p = 0; p <= 23; p++) {
        long msg_size = (p == 0) ? 0 : (1L << (p - 1));
        std::vector<char> buf(msg_size, 'x');

        MPI_Barrier(MPI_COMM_WORLD);
        double t1 = MPI_Wtime();

        for (int i = 0; i < n_iter; i++) {
            if (rank == 0) {
                MPI_Ssend(buf.data(), msg_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
                MPI_Recv(buf.data(), msg_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else if (rank == 1) {
                MPI_Recv(buf.data(), msg_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Ssend(buf.data(), msg_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
            }
        }

        double t2 = MPI_Wtime();

        if (rank == 0) {
            double dt = t2 - t1;
            double latency = dt / (n_iter * 2.0);
            double mb_moved = (double)msg_size * n_iter * 2.0 / (1024.0 * 1024.0);
            double bandwidth = (msg_size == 0) ? 0.0 : mb_moved / dt;

            printf("%-10ld | %-7d | %-10.5f | %-12.9f | %-10.2f\n", 
                   msg_size, n_iter, dt, latency, bandwidth);
        }
    }

    MPI_Finalize();
    return 0;
}
