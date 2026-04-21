#include <stdio.h>
#include <omp.h>
#include <string.h>
#include <stdlib.h>
#define NUMBER_THREADS 3

typedef struct{
  int rows;
  int cols;
  double *data;
}matrix;

matrix create_matrix(int r , int c){
  matrix m;
  m.rows = r;
  m.cols = c;

  m.data = (double*)malloc(r*c*sizeof(double));
  return m;
}
void freeematrix(matrix m){
  free(m.data);
}


void print_matrix(matrix m) {
    for (int i = 0; i < m.rows; i++) {
        for (int j = 0; j < m.cols; j++) {
            printf("%10.6f ", m.data[i * m.cols + j]);
        }
        printf("\n");
    }
}

int main(){
  int N = 2000;
  int N_iter = 100;
  double temp = 0.0; 
  //scanf("%d %d",&N,&N_iter);


  matrix A = create_matrix(N,N);
  matrix x_old = create_matrix(N,1);
  matrix x_new = create_matrix(N,1);
  matrix b = create_matrix(N,1);
  // initial fillings wiht the zeros 
  memset(x_old.data,0, N * sizeof(double));  
  
  // matrix example samll
  //A.data[0*N + 0] = 10.0; A.data[0*N + 1] = 2.0;  A.data[0*N + 2] = 1.0;
  //A.data[1*N + 0] = 1.0;  A.data[1*N + 1] = 5.0;  A.data[1*N + 2] = 1.0;
  //A.data[2*N + 0] = 2.0;  A.data[2*N + 1] = 3.0;  A.data[2*N + 2] = 10.0;
  //b.data[0] = 7.0;
  //b.data[1] = -8.0;
  //b.data[2] = 6.0;
  //
  // matrix example big
  for (int i = 0; i < N; ++i) {
        b.data[i] = 5.0;
        x_old.data[i] = 0.0;
        for (int j = 0; j < N; ++j) {
            if (i == j) {
                A.data[i*N + j] = 5000.0; 
            } else {
                A.data[i*N + j] = 1.0;    
            }
        }
    }

  
  //algo Jacbi 
  double start_time = omp_get_wtime();
  omp_set_num_threads(NUMBER_THREADS);
  #pragma omp parallel
  {
  for(int g =0; g< N_iter ; ++g){
    // as i understand we need to somehow pull out a row and calculate per row to parallelize
    #pragma omp for private(temp)
    for(int i =0; i < N;++i){
      // we take the row i for example:
      // we have the x0 and we need to update it
      
      double temp = 0.0;
      for(int j = 0 ;j < i; j++){
        // lower raingle case: we take all the values below the diagonal
          temp -= (A.data[i*N + j]) * x_old.data[j];}
      
      for(int j = N- 1; j > i; --j){
        // upper here
           temp -=(A.data[i*N + j]) * x_old.data[j];}
      x_new.data[i] = (1.0 / A.data[i*N + i]) * (b.data[i] + temp);
      }
    #pragma omp single
    {
    double *temp_ptr = x_old.data;
    x_old.data = x_new.data;
    x_new.data = temp_ptr;
    }
    
  }
  }
  printf("It took seconds:%f\n" , omp_get_wtime() - start_time);
  //printf("The final answer x\n");
  //print_matrix(x_old);
 return 0;
}
