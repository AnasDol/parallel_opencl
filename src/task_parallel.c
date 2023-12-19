#include <CL/cl.h>
#include <stdio.h>

#define MAX_SOURCE_SIZE (0x100000)

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

    int result[4]; // будущий результат выполнения
    for (int i = 0; i < 4; i++) {
        result[i] = -1;
    }

    // Загрузка кода ядра из файла
    FILE *kernelFile = fopen("test.cl", "r");
    if (!kernelFile) {
        fprintf(stderr, "Не удалось открыть файл с кодом ядра.\n");
        return 1;
    }

    char *kernelSource = (char *)malloc(MAX_SOURCE_SIZE);
    size_t kernelSize = fread(kernelSource, 1, MAX_SOURCE_SIZE, kernelFile);
    fclose(kernelFile);

    // Получение доступных платформ
    cl_uint numPlatforms;
    clGetPlatformIDs(0, NULL, &numPlatforms);

    cl_platform_id *platforms = (cl_platform_id *)malloc(numPlatforms * sizeof(cl_platform_id));
    clGetPlatformIDs(numPlatforms, platforms, NULL);

    // Выбор первой платформы
    cl_platform_id platform = platforms[0];

    // Получение доступных устройств
    cl_uint numDevices;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);

    cl_device_id *devices = (cl_device_id *)malloc(numDevices * sizeof(cl_device_id));
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);

    // Выбор первого устройства
    cl_device_id device = devices[0];

    // Создание контекста
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);

    // Создание командной очереди
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);



    // Буферы для массива простых чисел и результата
    cl_mem primes_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * count, primes, NULL);
    cl_mem result_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * 4, result, NULL);

    // Загрузка программы в контекст
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, &kernelSize, NULL);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    // Создание ядра
    cl_kernel kernel = clCreateKernel(program, "find_max", NULL);

    // Установка параметров ядра
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &primes_buffer);
    clSetKernelArg(kernel, 1, sizeof(int), &count);
    clSetKernelArg(kernel, 2, sizeof(int), &N);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &result_buffer);

    // Выполнение программы на устройстве
    size_t globalWorkSize = count*count*count;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalWorkSize, NULL, 0, NULL, NULL);

    // Получение результатов
    clEnqueueReadBuffer(queue, result_buffer, CL_TRUE, 0, sizeof(int)  * 4, result, 0, NULL, NULL);


    printf("\n---------------Output----------------\n");

    printf("%d^2 + %d^3 + %d^4 = %d\n", result[1], result[2], result[3], result[0]);
    //printf("Time, sec: %lf\n", finish - start);
    
    // Освобождение ресурсов
    clReleaseMemObject(primes_buffer);
    clReleaseMemObject(result_buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    free(kernelSource);
    free(platforms);
    free(devices);
    free(primes);

    return 0;
}