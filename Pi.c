#include <stdio.h>
#include <omp.h>
#define NUMBER_THREADS 8
int main()
{
    const size_t N = 1000000000;
    double step;
    double start_time, end_time;

    double x, pi, sum = 0.;

    step = 1. / (double)N;
    //
    start_time = omp_get_wtime();
    //
    
    omp_set_num_threads(NUMBER_THREADS);

    #pragma omp parallel reduction(+:sum)// we accumulate the sum for all the threads  
      #pragma omp for private(x)
        for (int i = 0; i < N; ++i)
          {
            x = (i + 0.5) * step;
            sum += 4.0 / (1. + x * x);
          }
    pi = sum * step;

    //
    end_time = omp_get_wtime();
    //
    printf("Work took %f seconds\n", end_time - start_time);
    printf("pi = %.16f\n", pi);

    return 0;
}
