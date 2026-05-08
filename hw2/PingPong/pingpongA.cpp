#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int max_bounce_times = 10;

struct PingPong_struct {
    int bounces_;
    int max_bounces_;
    int v_[max_bounce_times + 1];
};

int generate_next_guy(int current_guy, int psize) {
    int target;
    do {
        target = rand() % psize;
    } while (target == current_guy);
    return target;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int prank;
    int psize;

    MPI_Comm_rank(MPI_COMM_WORLD, &prank);
    MPI_Comm_size(MPI_COMM_WORLD, &psize);
    srand(time(NULL) + prank);

    PingPong_struct PingPong;

    int block_lengths[3] = {1, 1, max_bounce_times + 1};
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint offsets[3];
    MPI_Aint base_address, addr1, addr2, addr3;

    MPI_Get_address(&PingPong, &base_address);
    MPI_Get_address(&PingPong.bounces_, &addr1);
    MPI_Get_address(&PingPong.max_bounces_, &addr2);
    MPI_Get_address(&PingPong.v_[0], &addr3);

    offsets[0] = addr1 - base_address;
    offsets[1] = addr2 - base_address;
    offsets[2] = addr3 - base_address;

    MPI_Datatype MPI_BALL_TYPE;
    MPI_Type_create_struct(3, block_lengths, offsets, types, &MPI_BALL_TYPE);
    MPI_Type_commit(&MPI_BALL_TYPE);

    if (prank == 0) {
        PingPong.bounces_ = 1;                         
        PingPong.max_bounces_ = max_bounce_times;        
        PingPong.v_[0] = prank;                         

        int generated_target = generate_next_guy(prank, psize);
        MPI_Ssend(&PingPong, 1, MPI_BALL_TYPE, generated_target, 0, MPI_COMM_WORLD);
    }

    bool playing = true;
    MPI_Status status;

    while (playing) {

        MPI_Recv(&PingPong, 1, MPI_BALL_TYPE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == 1) {
            playing = false;
        } 
        else if (status.MPI_TAG == 0) {
            
            if (PingPong.bounces_ >= PingPong.max_bounces_) {
                PingPong.v_[PingPong.bounces_] = prank;

                printf("this is process number %d. The path was: ", prank);
                for(int i = 0; i < PingPong.max_bounces_; i++){
                    printf("%d ", PingPong.v_[i]);
                }
                printf("\n");

                playing = false; 
                for (int i = 0; i < psize; i++) {
                    if (i != prank) {
                        MPI_Ssend(&PingPong, 1, MPI_BALL_TYPE, i, 1, MPI_COMM_WORLD);
                    }
                }
            } 
            else {
                PingPong.v_[PingPong.bounces_] = prank;
                
                
                PingPong.bounces_++;
                
                
                int generated_target = generate_next_guy(prank, psize);
                
                
                MPI_Ssend(&PingPong, 1, MPI_BALL_TYPE, generated_target, 0, MPI_COMM_WORLD);
            }
        }
    }
    MPI_Type_free(&MPI_BALL_TYPE);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}
