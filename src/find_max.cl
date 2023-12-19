__kernel void find_max(__global int* primes, int count, int N, __global int* resultBuffer) {
    int gid = get_global_id(0);
    int totalThreads = get_global_size(0);

    for (int i = gid; i < count; i += totalThreads) {
        for (int j = 0; j < count; j++) {
            for (int k = 0; k < count; k++) {
                int max = primes[i] * primes[i] + primes[j] * primes[j] * primes[j] + primes[k] * primes[k] * primes[k] * primes[k];
                if (max <= N) {
                    atomic_max(resultBuffer, max);
                }
            }
        }
    }
}