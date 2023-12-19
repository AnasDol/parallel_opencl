__kernel void find_max(__global int* primes, int count, int N, __global int* sum_array, __global int* result) {

    int gid = get_global_id(0);

    if (gid < count*count*count) {

        int base = count; // Основание новой системы счисления
        int index1, index2, index3;

        index1 = gid / (base * base);
        index2 = (gid / base) % base;
        index3 = gid % base;

        int sum = primes[index1]*primes[index1] 
            + primes[index2]*primes[index2]*primes[index2] 
            + primes[index3]*primes[index3]*primes[index3]*primes[index3];

        sum_array[gid] = sum;

        if (sum > result[0] && sum < N) {

            atomic_max(&result[0], sum);

            if (sum == result[0]) {
                atomic_xchg(&result[1], primes[index1]);
                atomic_xchg(&result[2], primes[index2]);
                atomic_xchg(&result[3], primes[index3]);
            }

        }

    }

}