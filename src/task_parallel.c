#include <CL/cl.h>
#include <stdio.h>

#define MAX_SOURCE_SIZE (0x100000)

int main() {

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

    int N = 30;

    int count = 4;
    int* primes = (int*) malloc (sizeof(int) * count);
    primes[0] = 2;
    primes[1] = 3;
    primes[2] = 5;
    primes[3] = 7;
    cl_mem primes_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * count, primes, NULL);

    int* sum = (int*) malloc (sizeof(int) * count*count*count);
    cl_mem sum_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * count*count*count, sum, NULL);

    int result[4];
    for (int i = 0; i < 4; i++) {
        result[i] = -1;
    }
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
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &sum_buffer);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), &result_buffer);

    // Выполнение программы на устройстве
    size_t globalWorkSize = count*count*count;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalWorkSize, NULL, 0, NULL, NULL);

    // Получение результатов
    clEnqueueReadBuffer(queue, sum_buffer, CL_TRUE, 0, sizeof(int)  * count*count*count, sum, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, result_buffer, CL_TRUE, 0, sizeof(int)  * 4, result, 0, NULL, NULL);

    printf("Original array: \n");
    for (int i = 0; i < count; i++) {
        printf("%d ", primes[i]);
    }
    printf("\n\n");

    printf("sum array: \n");
    for (int i = 0; i < count*count*count; i++) {
        printf("%d ", sum[i]);
    }
    printf("\n\n");

    printf("result array: \n");
    for (int i = 0; i < 4; i++) {
        printf("%d ", result[i]);
    }

    // Освобождение ресурсов
    clReleaseMemObject(primes_buffer);
    clReleaseMemObject(sum_buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    free(kernelSource);
    free(platforms);
    free(devices);
    free(sum);
    free(primes);

    return 0;
}