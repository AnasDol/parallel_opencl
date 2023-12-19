__kernel void find_max(__global int* comb, int count) {

    int gid = get_global_id(0);

    if (gid < count) {

        int prime1 = comb[gid * 3 + 0];
        int prime2 = comb[gid * 3 + 1];
        int prime3 = comb[gid * 3 + 2];

        int sum = prime1*prime1 + prime2*prime2*prime2 + prime3*prime3*prime3*prime3;

        comb[gid * 3] = sum;

    }


}