#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
float sum = 0.0;
float dotprod(float * a, float * b, size_t N)
{
    int i, tid;


    tid = omp_get_thread_num(); 
 
    #pragma omp for reduction(+:sum)   
    for (i = 0; i < N; ++i) 
    {
        sum += a[i] * b[i];
        printf("tid = %d i = %d\n", tid, i);

    }

    return sum;
}

int main (int argc, char *argv[])
  // i guess the error is that the sum variable is not accessible in the function as it lives in the main
  // so i just moved if out and made it global (as c doesn't have a way to show that sun is there )
{
    const size_t N = 10000;
    int i;
    float a[N], b[N];


    for (i = 0; i < N; ++i)
    {
        a[i] = b[i] = (double)i;
    }


    #pragma omp parallel
    {
    dotprod(a, b, N);
    }
    printf("Sum = %f\n",sum);

    return 0;
}
