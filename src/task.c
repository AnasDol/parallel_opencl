#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <omp.h>

typedef struct {
    int max;
    int a, b, c;
} Result;

Result find_max(int* primes, int count, int N) {
    
    Result result;
    result.max = -1;
    result.a = -1;
    result.b = -1;
    result.c = -1;

    if (N < 28) return result;

    for (int i = 0; i < count; i++) {
        for (int j = 0; j < count; j++) {
            for (int k = 0; k < count; k++) {
                int max = primes[i]*primes[i] + primes[j]*primes[j]*primes[j] + primes[k]*primes[k]*primes[k]*primes[k];
                if (max <= N && max > result.max) {
                    result.max = max;
                    result.a = primes[i];
                    result.b = primes[j];
                    result.c = primes[k];
                }
            }
        }
    }
    return result;
}

int sieve_of_eratosthenes(int* numbers, int N) {

    if (N < 28) return -1;

    numbers[0] = 0;
    numbers[1] = 0;

    for (int i = 2; i <= N; ++i) {
        numbers[i] = 1;
    }

    int count = N-1; // число простых чисел

    // Начинаем проверку с 2 (первое простое число)
    for (int i = 2; i * i <= N; ++i) {
        if (numbers[i]) {
            // Отмечаем все кратные числа i как непростые
            for (int j = i * i; j <= N; j += i) {
                if (numbers[j] == 1) count--;
                numbers[j] = 0;
            }
        }
    }

    return count;

}

int main(int argc, char* argv[]) {

    int N;
    printf("N = ", N);
    if (!(scanf("%d", &N)==1 && N>=1)) {
        printf("Input error\n");
        return 0;
    }

    if (N < 28) {
        printf("Too small number. MIN = 28.");
        return 0;
    }

    int* numbers = (int*) malloc (sizeof(int) * N+1);  // будущий массив нулей и единиц

    double start = omp_get_wtime();

    int count = sieve_of_eratosthenes(numbers, N);
    printf("count: %d\n", count); // количество простых чисел

    int* primes = (int*) malloc (sizeof(int) * count); // массив простых чисел

    for (int i = 0, j = 2; i < count, j < N + 1; j++) {
        // если число простое, записываем в массив
        if (numbers[j]) {
            primes[i] = j;
            i++;
        }
    }

    if (count <= 30) {
        printf("primes: ");
        for (int i = 0;i<count;i++) {
            printf("%d ", primes[i]);
        }
    }

    Result result = find_max(primes, count, N);

    double finish = omp_get_wtime();

    printf("\n---------------Output----------------\n");

    printf("%d^2 + %d^3 + %d^4 = %d\n", result.a, result.b, result.c, result.max);
    printf("Time, sec: %lf\n", finish - start);
    
    free(numbers);
    free(primes);
    
    return 0;
}