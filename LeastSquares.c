#include <stdio.h>
#include <omp.h>
#include <time.h>

typedef struct{
  double x;
  double y;
} point;

typedef struct{
  int n;
  point *data;
} dataset;

dataset create_dataset(int N){
  dataset df;
  df.n = N;

  df.data = (point*)malloc(N * sizeof(point));

  return df;
}



dataset fill_dataset(int N, double a , double b){  
  dataset df = create_dataset(N);
  int range_start = 0;


  srand(time(NULL));
  int x = rand();

  int range_end = 1; 
  double step = 1.0 / N;
  for(int i =0 ; i < N; ++i){
    double noisce = (((double)rand()/ RAND_MAX) - 0.5)*0.1;
    df.data[i].x = i *step;
    df.data[i].y = (a * df.data[i].x) + b + noisce;
    
  }
  return df;
}

int main(){
  int N=10;
  int N_iter = 1000;
  double a = 1;
  double b = 2;
  double a_old = 0.0;
  double b_old = 0.0;
  double learning_rate = 0.05;
  dataset df = fill_dataset(N ,a,b);
  // if you wanto check you the dataset
  //for(int i= 0; i < N;++i){
  //  printf("x:%f, y:%f" ,df.data[i].x, df.data[i].y);
  //}
  for(int i = 0; i < N_iter; ++i){
    double temp_1 = 0.0;
    double temp_2 = 0.0;
    #pragma omp parallel for reduction(+:temp_1 , temp_2)
    for(int j =0; j < N; ++j){
      temp_1 += (df.data[j].x * a_old + b_old - df.data[j].y) * df.data[j].x;
      temp_2 += (df.data[j].x * a_old + b_old - df.data[j].y);
      //int tid = omp_get_thread_num();
      //printf("temp1:%f, temp2:%f , tid:%d\n" , temp_1, temp_2, tid);
    }
      a_old = a_old - (learning_rate *temp_1)/df.n;
      b_old = b_old - (learning_rate *temp_2)/df.n;
  }
  printf("a:%f\nb:%f" , a_old, b_old);
}
