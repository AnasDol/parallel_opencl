__kernel void find_max(__global int* primes, int count, int N, __global int* result_array) {

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

        result_array[gid] = sum;

    }

}