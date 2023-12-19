__kernel void square(__global int* input, __global int* globalMax) {
    int gid = get_global_id(0);
    int squaredValue = input[gid] * input[gid];

    // Сравнение с глобальным максимальным значением
    if (squaredValue > *globalMax) {
        // Обновление глобального максимального значения
        atomic_max(globalMax, squaredValue);
    }

    // Запись квадрата значения обратно в массив
    input[gid] = squaredValue;
}
